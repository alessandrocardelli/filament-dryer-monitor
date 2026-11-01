#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Update.h>
#include <ArduinoOTA.h>
#include <esp_system.h>
#include "FirmwareBuild.h"
#include "ThermalLog.h"
#include "HeaterPlateDiagnostics.h"
#include "HeaterStep07GuardTrial.h"
#include "HeaterStep09AirFeedback.h"

// Local-only read-only telemetry + browser OTA. No remote heater controls.
// Provisioning AP uses a fresh random WPA2 key displayed on the OLED.
// Network failure is never a reason to change the local safety/cycle state.
class NetworkService {
 public:
  struct Status {
    uint32_t ms = 0;
    float airC = 0, rh = 0, ntcOhms = 0;
    bool airValid = false, ntcValid = false;
    bool heaterCommand = false, cycle = false, fault = false;
    uint8_t fanPercent = 0, phase = 0, pulses = 0, pulseFault = 0, waitReason = 0;
    uint32_t cumulativeHeatMs = 0;
    bool sessionCsvSaved = false, sessionCsvFinal = false;
    bool logTruncated = false;
    float minSessionNtcOhms = NAN; // measured NTC, not a Sanwa temperature
  };
  explicit NetworkService(ThermalLog &log) : log_(log) {}

  void begin() {
    if (prefs_.begin("dryer_net", false)) {
      ssid_ = prefs_.getString("ssid", "");
      wifiPass_ = prefs_.getString("wifi", "");
      adminPass_ = prefs_.getString("admin", "");
    } else {
      Serial.println("WiFi NVS unavailable");
    }
    WiFi.persistent(false);
    if (ssid_.length() && adminPass_.length() >= 12) {
      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      WiFi.begin(ssid_.c_str(), wifiPass_.c_str());
      connectingSinceMs_ = millis();
      Serial.println("WiFi connecting to configured network");
    } else {
      startSetupAp();
    }
    configureRoutes();
    server_.begin();
  }

  void publish(const Status &s) { live_ = s; }

  // Owned by application; called synchronously before the IDE OTA transfer.
  // Hard-off safety controller and stop fan even though standby is required.
  void setIdeOtaStartSafetyCallback(void (*callback)()) {
    ideOtaStartSafetyCallback_ = callback;
  }

  // Small RAM diagnostic console. Mirrors relevant state/fault transitions,
  // not arbitrary strings or credentials from Serial.
  void note(const char *message) {
    snprintf(events_[eventHead_], sizeof(events_[eventHead_]),
             "[%lu ms] %s", (unsigned long)millis(), message);
    eventHead_ = (eventHead_ + 1) % kEventCapacity;
    if (eventCount_ < kEventCapacity) ++eventCount_;
  }

  void tick(uint32_t nowMs, bool permitHttp, bool permitOta) {
    otaAllowed_ = permitOta && !uploadActive_ && !rebootPending_ &&
                  !ideOtaBusy_;
    if (ssid_.length() && WiFi.status() == WL_CONNECTED) {
      if (!staAnnounced_) {
        staAnnounced_ = true;
        mdnsStarted_ = MDNS.begin("filament-dryer");
        if (mdnsStarted_) MDNS.addService("http", "tcp", 80);
        Serial.print("WiFi address: http://");
        Serial.println(WiFi.localIP());
        note("WIFI CONNECTED");
      }
      if (!ideOtaStarted_) startIdeOta();
      if (setupApActive_) {
        WiFi.softAPdisconnect(true);
        setupApActive_ = false;
      }
    } else {
      if (staAnnounced_) {
        if (ideOtaStarted_) {
          ArduinoOTA.end();
          ideOtaStarted_ = false;
        }
        if (mdnsStarted_) {
          MDNS.end();
          mdnsStarted_ = false;
        }
      }
      staAnnounced_ = false;
      if (ssid_.length() && !setupApActive_ &&
          nowMs - connectingSinceMs_ >= 15000) startSetupAp();
    }

    // ArduinoOTA.handle() may block while receiving an entire application.
    // It is NEVER serviced during a cycle or a heater/cooldown/fault state.
    // An already authenticated upload finishes without running the loop.
    if (permitOta && ideOtaStarted_ && !uploadActive_ &&
        !rebootPending_ && !ideOtaBusy_) {
      ArduinoOTA.handle();
    }

    // A synchronous HTTP request must never extend a heater pulse.
    if ((permitHttp || uploadActive_) && !ideOtaBusy_)
      server_.handleClient();
    if (rebootPending_ && (int32_t)(nowMs - rebootAtMs_) >= 0) {
      ESP.restart();
    }
  }

  bool uploadActive() const { return uploadActive_; }
  bool updateBusy() const {
    return uploadActive_ || rebootPending_ || ideOtaBusy_;
  }
  bool ideOtaReady() const { return ideOtaStarted_ && otaAllowed_; }
  bool setupApActive() const { return setupApActive_; }
  const char *setupSsid() const { return apSsid_.c_str(); }
  const char *setupPass() const { return apPass_.c_str(); }
  String address() const {
    if (WiFi.status() == WL_CONNECTED) return WiFi.localIP().toString();
    if (setupApActive_) return WiFi.softAPIP().toString();
    return "connecting";
  }
  bool connected() const { return WiFi.status() == WL_CONNECTED; }
  const char *controlPasswordForLocalServer() const { return adminPass_.c_str(); }

 private:
  static constexpr size_t kEventCapacity = 32;
  char events_[kEventCapacity][88]{};
  size_t eventHead_ = 0, eventCount_ = 0;
  ThermalLog &log_;
  WebServer server_{80};
  Preferences prefs_;
  Status live_{};
  String ssid_, wifiPass_, adminPass_, apSsid_, apPass_;
  bool setupApActive_ = false, mdnsStarted_ = false, staAnnounced_ = false;
  bool ideOtaStarted_ = false, ideOtaBusy_ = false;
  void (*ideOtaStartSafetyCallback_)() = nullptr;
  bool otaAllowed_ = false, uploadActive_ = false, updateOk_ = false;
  bool rebootPending_ = false;
  uint32_t rebootAtMs_ = 0, connectingSinceMs_ = 0;

  void startIdeOta() {
    if (ideOtaStarted_ || adminPass_.length() < 12 ||
        WiFi.status() != WL_CONNECTED) return;

    ArduinoOTA.setPort(3232);
    ArduinoOTA.setHostname("filament-dryer");
    ArduinoOTA.setPassword(adminPass_.c_str());
    // NetworkService owns mDNS for HTTP and IDE: don't call MDNS.begin()
    // twice. Advertise _arduino._tcp explicitly on the existing responder.
    ArduinoOTA.setMdnsEnabled(false);
    ArduinoOTA.onStart([this]() {
      ideOtaBusy_ = true;
      if (ideOtaStartSafetyCallback_) ideOtaStartSafetyCallback_();
      note("IDE OTA START (HEATER OFF)");
      Serial.println("IDE OTA accepted in idle; heater forced OFF");
    });
    ArduinoOTA.onEnd([this]() {
      note("IDE OTA COMPLETE");
      Serial.println("IDE OTA COMPLETE; reboot");
      // Keep the button interlock latched until the core reboots.
    });
    ArduinoOTA.onError([this](ota_error_t code) {
      char msg[48];
      snprintf(msg, sizeof(msg), "IDE OTA ERROR %u", (unsigned)code);
      note(msg);
      Serial.println(msg);
      ideOtaBusy_ = false;  // old firmware stays active if update failed
    });
    ArduinoOTA.begin();
    ideOtaStarted_ = true;
    if (mdnsStarted_) MDNS.enableArduino(3232, true);
    Serial.println("IDE OTA enabled: filament-dryer.local:3232");
    note("IDE OTA READY");
  }

  void startSetupAp() {
    if (setupApActive_) return;
    char password[13];
    snprintf(password, sizeof(password), "%08lX%04lX",
             (unsigned long)esp_random(),
             (unsigned long)(esp_random() & 0xFFFFUL));
    apPass_ = password;
    const uint64_t mac = ESP.getEfuseMac();
    char shortName[12];
    snprintf(shortName, sizeof(shortName), "%04X",
             (unsigned)(mac & 0xFFFFu));
    apSsid_ = String("FilamentDryer-") + shortName;
    WiFi.mode(ssid_.length() ? WIFI_AP_STA : WIFI_AP);
    setupApActive_ = WiFi.softAP(apSsid_.c_str(), apPass_.c_str());
    Serial.print("WiFi SETUP SSID: "); Serial.println(apSsid_);
    Serial.print("WiFi SETUP key (OLED): "); Serial.println(apPass_);
    Serial.println("Setup address: http://192.168.4.1");
  }

  bool authenticated() {
    if (adminPass_.length() < 12) return false;
    return server_.authenticate("admin", adminPass_.c_str());
  }
  void refuseAuth() {
    server_.requestAuthentication(BASIC_AUTH, "Filament Dryer OTA");
  }
  bool safeForOta() const {
    return otaAllowed_ && !ideOtaBusy_ &&
           !live_.heaterCommand && !live_.cycle && !live_.fault;
  }

  void configureRoutes() {
    server_.on("/", HTTP_GET, [this]() {
      if (setupApActive_ && WiFi.status() != WL_CONNECTED) {
        server_.sendHeader("Location", "/setup");
        server_.send(302, "text/plain", "Setup");
        return;
      }
      // HTML carries inlined JavaScript. Do not let the browser keep a prior
      // firmware's pulse-count UI across OTA upgrades.
      server_.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
      server_.send(200, "text/html; charset=utf-8", dashboardHtml());
    });
    server_.on("/setup", HTTP_GET, [this]() {
      if (!setupApActive_) { server_.send(403, "text/plain", "Setup AP inactive"); return; }
      server_.send(200, "text/html; charset=utf-8",
        "<!doctype html><meta name='viewport' content='width=device-width'>"
        "<h2>Filament Dryer - WiFi setup</h2>"
        "<p>Connect to a trusted 2.4 GHz network. Set a separate OTA password "
        "(12+ characters); store it securely. The password is not published on GitHub.</p>"
        "<form method='post' action='/save'>"
        "<label>WiFi SSID <input required name='ssid' maxlength='32'></label><br>"
        "<label>WiFi password <input name='pass' type='password' maxlength='64'></label><br>"
        "<label>OTA password <input name='admin' type='password' minlength='12' maxlength='63' required></label><br>"
        "<button type='submit'>Save and restart</button></form>");
    });
    server_.on("/save", HTTP_POST, [this]() {
      if (!setupApActive_) { server_.send(403, "text/plain", "Not in provisioning mode"); return; }
      const String ssid = server_.arg("ssid"), pass = server_.arg("pass"), admin = server_.arg("admin");
      if (ssid.isEmpty() || ssid.length() > 32 || pass.length() > 64 ||
          admin.length() < 12 || admin.length() > 63) {
        server_.send(400, "text/plain", "Invalid SSID or password sizes"); return;
      }
      const size_t savedSsid = prefs_.putString("ssid", ssid);
      const size_t savedWifi = prefs_.putString("wifi", pass);
      const size_t savedAdmin = prefs_.putString("admin", admin);
      if (!savedSsid || (!savedWifi && !pass.isEmpty()) || !savedAdmin) {
        server_.send(500, "text/plain", "NVS write failed"); return;
      }
      server_.send(200, "text/html",
        "<h2>Saved</h2><p>ESP32 restarting. Reconnect your computer/phone "
        "to your normal WiFi and visit http://filament-dryer.local "
        "or read the IP on the OLED.</p>");
      scheduleReboot();
    });
    server_.on("/api/status", HTTP_GET, [this]() {
      String j; j.reserve(430);
      j += "{\"firmware\":\""; j += FirmwareBuild::kId;
      j += "\",\"ms\":"; j += live_.ms;
      j += ",\"air_c\":"; j += String(live_.airC,2);
      j += ",\"rh\":"; j += String(live_.rh,2);
      j += ",\"ntc_ohms\":"; j += String(live_.ntcOhms,1);
      // Read-only measured margins; historical Sanwa proxy is intentionally
      // marked separately and never drives PWM or any safety trip.
      const auto trialLimits = AppConfig::kHeaterStep09TrialMode
          ? HeaterStep09AirFeedback::attendedSanwa90_85Parameters()
          : HeaterStep09AirFeedback::parameters();
      const auto margin = HeaterPlateDiagnostics::margin(
          live_.ntcValid ? live_.ntcOhms : NAN,
          trialLimits.ntcSoftPauseOhms, trialLimits.ntcHardTripOhms);
      const auto historical = HeaterPlateDiagnostics::historicalOpenBoxContact(
          live_.ntcValid ? live_.ntcOhms : NAN);
      j += ",\"ntc_soft_pause_ohms\":"; j += String(trialLimits.ntcSoftPauseOhms,1);
      j += ",\"ntc_resume_ohms\":"; j += String(trialLimits.ntcResumeOhms,1);
      j += ",\"ntc_backup_trip_ohms\":"; j += String(trialLimits.ntcHardTripOhms,1);
      j += ",\"ntc_above_soft_ohms\":";
      j += margin.valid ? String(margin.aboveSoftOhms,1) : "null";
      j += ",\"ntc_above_hard_ohms\":";
      j += margin.valid ? String(margin.aboveHardOhms,1) : "null";
      j += ",\"session_ntc_min_ohms\":";
      j += std::isfinite(live_.minSessionNtcOhms)
           ? String(live_.minSessionNtcOhms,1) : "null";
      j += ",\"historical_openbox_sanwa_proxy_c\":";
      j += historical.valid ? String(historical.sanwaEquivalentC,1) : "null";
      j += ",\"air_valid\":"; j += (live_.airValid ? "true" : "false");
      j += ",\"ntc_valid\":"; j += (live_.ntcValid ? "true" : "false");
      j += ",\"heater_command\":"; j += (live_.heaterCommand ? "true" : "false");
      j += ",\"cycle\":"; j += (live_.cycle ? "true" : "false");
      j += ",\"fault\":"; j += (live_.fault ? "true" : "false");
      j += ",\"fan_percent\":"; j += live_.fanPercent;
      j += ",\"phase\":"; j += live_.phase;
      j += ",\"pulses\":"; j += live_.pulses;
      j += ",\"pulse_goal\":"; j += (AppConfig::kHeaterStep09TrialMode ? 0 : HeaterStep07GuardTrial::parameters().pulseCount);
      j += ",\"pulse_fault\":"; j += live_.pulseFault;
      j += ",\"wait_reason\":"; j += live_.waitReason;
      j += ",\"cumulative_heat_ms\":"; j += live_.cumulativeHeatMs;
      j += ",\"session_csv_saved\":"; j += (live_.sessionCsvSaved ? "true":"false");
      j += ",\"session_csv_final\":"; j += (live_.sessionCsvFinal ? "true":"false");
      j += ",\"saved_log\":"; j += (log_.hasSaved() ? "true" : "false");
      j += ",\"ram_samples\":"; j += (unsigned)log_.size();
      j += ",\"log_truncated\":"; j += (live_.logTruncated ? "true" : "false");
      j += ",\"ota_idle\":"; j += (safeForOta() ? "true" : "false");
      j += ",\"ide_ota_ready\":"; j += (ideOtaReady() ? "true" : "false");
      j += "}";
      server_.sendHeader("Cache-Control", "no-store");
      server_.send(200,"application/json",j);
    });
    server_.on("/api/events", HTTP_GET, [this]() {
      server_.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server_.send(200, "text/plain; charset=utf-8", "");
      for (size_t i = 0; i < eventCount_; ++i) {
        const size_t ix = (eventHead_ + kEventCapacity - eventCount_ + i) %
                          kEventCapacity;
        server_.sendContent(String(events_[ix]) + "\\n");
      }
      server_.sendContent("");
    });
    server_.on("/api/history.csv", HTTP_GET, [this]() {
      // S9B AirStale at 575852ms coincided with increasing multi-second loop
      // gaps. Never synchronously stream an expanding RAM CSV during a trial
      // or fan cooldown; status/events are short, bounded endpoints.
      if (live_.cycle || live_.fanPercent > 0) {
        server_.send(409, "text/plain", "History available only after cooling");
        return;
      }
      server_.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server_.send(200, "text/csv", "");
      server_.sendContent(log_.header());
      char line[192];
      for (size_t i=0; i<log_.size(); ++i) {
        const auto &s = log_.at(i);
        snprintf(line,sizeof(line),"%lu,%.2f,%.2f,%.1f,%u,%u,%u,%u,%u,%u,%u,%u,%lu,%lu,%lu\n",
                 (unsigned long)s.ms,s.airC,s.humidity,s.ntcOhms,
                 (unsigned)(s.valid&1),(unsigned)((s.valid>>1)&1),
                 (unsigned)s.phase,(unsigned)s.pulses,
                 (unsigned)s.heater,(unsigned)s.fan,
                 (unsigned)s.pulseFault,(unsigned)s.ntcFault,
                 (unsigned long)s.airAgeMs,(unsigned long)s.ntcAgeMs,
                 (unsigned long)s.loopGapMs);
        server_.sendContent(line);
      }
      server_.sendContent("");
    });
    server_.on("/api/last-test.csv", HTTP_GET, [this]() {
      if (live_.cycle || live_.fanPercent > 0) {
        server_.send(409, "text/plain", "Download after cooling"); return;
      }
      if (!log_.hasSaved()) { server_.send(404,"text/plain","No saved test yet"); return; }
      File f = LittleFS.open("/last-test.csv","r");
      if (!f) { server_.send(500,"text/plain","Cannot open CSV"); return; }
      server_.sendHeader("Content-Disposition","attachment; filename=filament-last-test.csv");
      server_.streamFile(f,"text/csv");
      f.close();
    });
    server_.on("/ota", HTTP_GET, [this]() {
      if (!authenticated()) { refuseAuth(); return; }
      if (!safeForOta()) { server_.send(409,"text/plain","OTA only in idle STANDBY, heater OFF"); return; }
      server_.send(200,"text/html",
        "<!doctype html><meta name='viewport' content='width=device-width'>"
        "<h2>Filament Dryer - firmware OTA</h2>"
        "<p>Only upload an ESP32 Arduino application .bin compiled for this "
        "board with an OTA-enabled partition scheme. Never upload a full flash image.</p>"
        "<form method='POST' action='/ota' enctype='multipart/form-data'>"
        "<input type='file' name='firmware' accept='.bin' required>"
        "<button type='submit'>Install firmware</button></form>");
    });
    server_.on("/ota", HTTP_POST,
      [this]() {
        if (!authenticated()) { refuseAuth(); return; }
        if (!updateOk_ || Update.hasError()) {
          server_.send(500,"text/plain","Firmware update rejected or failed; old firmware retained");
        } else {
          server_.send(200,"text/plain","OTA completed. Rebooting...");
          scheduleReboot();
        }
        uploadActive_=false; updateOk_=false;
      },
      [this]() {
        HTTPUpload &u = server_.upload();
        if (u.status == UPLOAD_FILE_START) {
          updateOk_ = authenticated() && safeForOta();
          if (updateOk_) {
            uploadActive_ = true;
            updateOk_ = Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH);
            if (!updateOk_) uploadActive_ = false;
          }
        } else if (u.status == UPLOAD_FILE_WRITE) {
          if (uploadActive_ && updateOk_ && Update.write(u.buf,u.currentSize) != u.currentSize) {
            updateOk_ = false;
            Update.abort();
          }
        } else if (u.status == UPLOAD_FILE_END) {
          if (uploadActive_ && updateOk_) updateOk_ = Update.end(true);
          uploadActive_ = false;
        } else if (u.status == UPLOAD_FILE_ABORTED) {
          if (uploadActive_) Update.abort();
          updateOk_ = false; uploadActive_ = false;
        }
      });
    server_.onNotFound([this]() {
      server_.send(404,"text/plain","Not found");
    });
  }

  void scheduleReboot() {
    rebootPending_=true;
    rebootAtMs_=millis()+1200;
  }

  static const char *dashboardHtml() {
    return R"HTML(<!doctype html><html lang="it"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#111e2c">
<title>Filament Dryer | Diagnostica</title>
<style>
:root{font-family:system-ui,-apple-system,"Segoe UI",sans-serif;color-scheme:light;
--ink:#172636;--muted:#607287;--line:#dfe7ed;--bg:#f0f4f7;--white:#fff;--teal:#087b80}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font-size:15px}
.shell{max-width:1040px;margin:auto;padding:24px 20px 56px}
.topbar{display:flex;justify-content:space-between;align-items:center;gap:18px;
padding:20px 25px;border-radius:19px;background:#111e2c;color:#fff;box-shadow:0 12px 34px #142a3b18}
.brand{display:flex;align-items:center;gap:14px;min-width:0}
.mark{width:47px;height:47px;display:grid;place-items:center;border-radius:13px;
background:#147c80;font-weight:800;letter-spacing:-.06em;font-size:20px;flex:none}
.brand strong{display:block;font-size:19px;letter-spacing:-.03em}
.brand small{display:block;color:#a8bbc8;margin-top:2px;font-size:12px}
.header-right{display:flex;align-items:center;gap:16px;min-width:0}
.network{color:#d5e5ed;font-size:12px;text-align:right;line-height:1.5;max-width:320px}
.back{font-weight:700;font-size:12px;color:#fff;text-decoration:none;border:1px solid #647688;
border-radius:9px;padding:10px 12px;white-space:nowrap}
.back:hover{background:#273c4c}
.section-title{display:flex;align-items:baseline;justify-content:space-between;gap:12px;
margin:28px 1px 13px}.section-title h2{font-size:16px;letter-spacing:-.025em;margin:0}
.section-title small{color:var(--muted);font-size:12px}
.metrics{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:13px}
.card{background:#fff;border:1px solid var(--line);border-radius:17px;box-shadow:0 4px 18px #23344609}
.metric{padding:20px;min-height:134px}
.metric.main{grid-column:span 2;background:#fff;position:relative;overflow:hidden}
.metric.main:before{content:"";position:absolute;right:-40px;top:-60px;width:150px;
height:150px;border-radius:50%;background:#e2f2f1}
.kicker{display:flex;align-items:center;gap:8px;color:var(--muted);font-size:12px;
font-weight:650;text-transform:uppercase;letter-spacing:.065em}
.glyph{display:inline-grid;place-items:center;width:23px;height:23px;border-radius:7px;
background:#e9f3f4;color:#087b80;font-size:13px}
.value{margin-top:13px;font-size:27px;line-height:1.16;font-weight:730;
letter-spacing:-.045em;font-variant-numeric:tabular-nums;position:relative;overflow-wrap:anywhere}
.main .value{font-size:46px}
.hint{color:var(--muted);font-size:12px;margin-top:9px;position:relative}
.panel{padding:22px}
.panel-head{display:flex;align-items:center;justify-content:space-between;gap:10px;
border-bottom:1px solid var(--line);padding-bottom:15px;margin-bottom:16px}
.panel-head h2{font-size:17px;letter-spacing:-.025em;margin:0}
.tag{border-radius:20px;background:#e1f4f1;color:#066a6f;font-weight:700;
font-size:11px;padding:6px 10px;white-space:nowrap}
.description{font-size:12px;line-height:1.6;color:var(--muted);margin:10px 0}
.linkrow{display:flex;flex-wrap:wrap;gap:9px;margin:15px 0}
.linkbutton{display:inline-flex;align-items:center;justify-content:center;
text-align:center;border:1px solid #d3e3e5;color:#096d73;background:#f3fafa;
text-decoration:none;font-weight:700;font-size:12px;border-radius:10px;min-height:42px;padding:9px 13px}
.linkbutton:hover{background:#e1f4f1}
.chartwrap{width:100%;overflow:hidden;background:#fafcfd;border:1px solid #e9eff3;
border-radius:12px;padding:12px 8px 4px}
canvas{display:block;width:100%;max-width:100%;height:auto}
.diagnostics{display:grid;grid-template-columns:1fr 1fr;gap:14px}
pre{white-space:pre-wrap;overflow-wrap:anywhere;max-height:290px;overflow:auto;
padding:15px;border-radius:11px;background:#f5f8fa;border:1px solid #e5edf2;
color:#304455;font-size:11px;line-height:1.55;font-variant-numeric:tabular-nums}
details{margin-top:15px}
summary{font-size:13px;font-weight:700;color:#096d73;cursor:pointer}
.panel-meta{font-size:12px;color:var(--muted);line-height:1.6;overflow-wrap:anywhere}
.footer{color:var(--muted);font-size:11px;line-height:1.65;margin-top:22px}
@media(max-width:750px){.shell{padding:12px 12px 32px}.topbar{padding:16px;border-radius:14px}
.metrics{grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}
.metric.main{grid-column:span 1}.metric{padding:16px;min-height:117px}
.main .value{font-size:32px}.value{font-size:23px}.panel{padding:17px}
.diagnostics{grid-template-columns:1fr}.header-right{gap:9px}.network{max-width:190px}}
@media(max-width:440px){.mark{width:39px;height:39px;font-size:17px}
.brand{gap:10px}.brand strong{font-size:16px}.brand small{font-size:10px}
.header-right{flex-direction:column-reverse;align-items:flex-end;gap:6px}
.network{font-size:10px;max-width:145px}.back{font-size:10px;padding:7px 9px}
.main .value{font-size:30px}.kicker{font-size:10px}}
</style></head><body><main class="shell">
<header class="topbar"><div class="brand"><span class="mark" aria-hidden="true">FD</span>
<div><strong>Filament Dryer</strong><small>DIAGNOSTICA / ESP32</small></div></div>
<div class="header-right"><div class="network" id="net" aria-live="polite">Connessione...</div>
<a id="fastdash" class="back" href="#">Pannello rapido</a></div></header>
<div class="section-title"><h2>Panoramica</h2><small>Telemetria in tempo reale</small></div>
<section class="metrics" aria-label="Misure essiccatore">
<div class="card metric main"><div class="kicker"><span class="glyph">°</span>Temperatura aria</div>
<div class="value" id="air">--</div><div class="hint">SHT45 · camera di essiccazione</div></div>
<div class="card metric main"><div class="kicker"><span class="glyph">%</span>Umidità relativa</div>
<div class="value" id="rh">--</div><div class="hint">SHT45 · camera di essiccazione</div></div>
<div class="card metric"><div class="kicker"><span class="glyph">Ω</span>NTC piastra</div>
<div class="value" id="ntc">--</div><div class="hint">Resistenza misurata</div></div>
<div class="card metric"><div class="kicker"><span class="glyph">H</span>Riscaldatore</div>
<div class="value" id="heat">--</div><div class="hint">Stato del comando</div></div>
<div class="card metric"><div class="kicker"><span class="glyph">F</span>Ventola</div>
<div class="value" id="fan">--</div><div class="hint">Percentuale comandata</div></div>
<div class="card metric"><div class="kicker"><span class="glyph">C</span>Fase del ciclo</div>
<div class="value" id="phase">--</div><div class="hint">Stato del controllo</div></div>
</section>
<div class="section-title"><h2>Storico e grafico</h2><small>Registro RAM e CSV</small></div>
<section class="card panel">
<div class="panel-head"><h2>Andamento temperatura aria</h2><span class="tag">SHT45</span></div>
<p class="description">Andamento dei campioni registrati. Il grafico si aggiorna quando il
servizio HTTP principale è disponibile; durante il riscaldamento lo storico può restare sospeso.</p>
<div class="chartwrap"><canvas id="trend" width="850" height="220" role="img"
aria-label="Grafico temperatura aria registrata nel tempo"></canvas></div>
<div class="linkrow">
<a class="linkbutton" href="/api/history.csv">Esporta campioni CSV</a>
<a class="linkbutton" href="/api/last-test.csv">Ultimo test salvato</a>
<a class="linkbutton" href="/ota">Aggiornamento OTA</a>
</div>
<div id="ver" class="panel-meta"></div>
<details><summary>Ultimi campioni CSV (dati grezzi)</summary>
<pre id="rows">...</pre></details>
</section>
<div class="section-title"><h2>Diagnostica</h2><small>Eventi e messaggi</small></div>
<section class="card panel">
<div class="panel-head"><h2>Console eventi</h2><span class="tag">LOG</span></div>
<pre id="events">...</pre>
<p class="description">I valori NTC sono resistenze, non temperature della piastra.
La temperatura AIR è quella del sensore SHT45. Il grafico non sostituisce le
protezioni locali del riscaldatore.</p>
</section>
<footer class="footer">Filament Dryer Monitor · Dati in sola lettura nella pagina diagnostica.
I comandi ciclo e STOP sono disponibili sul pannello rapido; gli aggiornamenti OTA
mantengono le limitazioni del firmware.</footer>
</main>
<script>
const $=x=>document.getElementById(x);
// During heater ON the device deliberately does not service HTTP. Polling
// must not treat an old OFF reply as proof that GPIO19 is still OFF.
let statusReceivedAt=0, statusBusy=false, trialActive=false;
function phaseLabel(d){
 const done=Number(d.pulses)||0, total=Number(d.pulse_goal)||0;
 switch(Number(d.phase)){
  case 1: return 'Preparazione ventola';
  case 2: return total ? 'Riscaldamento · impulso '+Math.min(done+1,total)+' di '+total : 'HEATER ON · feedback SHT45';
  case 3: return total ? 'Pausa dopo impulso '+done+' di '+total : 'HEATER OFF · recupero NTC / aria';
  case 4: return 'Raffreddamento';
  case 5: return 'Test completato';
  case 6: return 'Test interrotto';
  default: return 'Standby';
 }
}
function refreshTelemetryAge(){
 const elapsed=statusReceivedAt?Date.now()-statusReceivedAt:Infinity;
 if(elapsed>2500){
  $('heat').textContent='NON AGGIORNATO';
  $('fan').textContent='--';
  $('net').textContent=statusReceivedAt
   ?'Telemetria sospesa ('+Math.round(elapsed/1000)+' s senza aggiornamenti)'
   :'In attesa di dati';
 }
}
async function status(){
 if(statusBusy)return;
 statusBusy=true;
 const controller=new AbortController();
  const liveUrl=window.location.protocol+'//'+window.location.hostname+':8081/api/status';
 const timeout=setTimeout(()=>controller.abort(),1800);
 try{
  const r=await fetch(liveUrl,{cache:'no-store',signal:controller.signal});
  if(!r.ok)throw Error(r.status);
  const d=await r.json();
  trialActive=Boolean(d.cycle)||Number(d.fan_percent)>0;
  $('air').textContent=d.air_valid?d.air_c.toFixed(1)+' °C':'--';
  $('rh').textContent=d.air_valid?d.rh.toFixed(1)+' %':'--';
  $('ntc').textContent=d.ntc_valid
    ? (d.ntc_ohms/1000).toFixed(2)+' kΩ'
    :'--';
  $('heat').textContent=d.heater_command?'ON':'OFF';
  $('fan').textContent=d.fan_percent+' %';
  const waitNames=['','Minimum OFF 12 s','NTC attende 60 kΩ','Camera attende 24 °C'];
  $('phase').textContent=Number(d.phase)===3 && d.wait_reason
    ? 'HEATER OFF · '+(waitNames[Number(d.wait_reason)]||'Attesa controllo')
    :phaseLabel(d);
  $('ver').textContent='Firmware '+d.firmware+' | campioni '+d.ram_samples+
    (d.fault?' | FAULT '+d.pulse_fault:'')+
    ' | ON '+((d.cumulative_heat_ms||0)/1000).toFixed(1)+' s'+
    ' | minimo NTC '+(d.session_ntc_min_ohms===null?'--':
       (d.session_ntc_min_ohms/1000).toFixed(2)+' kΩ')+
    ' | CSV '+(d.log_truncated?'DATI PARZIALI':d.session_csv_final?'COMPLETO':d.session_csv_saved?'CHECKPOINT':'non salvato in questa sessione')+
    ' | '+(d.ota_idle?'OTA pronto':'OTA non disponibile');
  statusReceivedAt=Date.now();
  $('net').textContent='Dati aggiornati';
 }catch(e){
  // On an intentional HTTP pause, never keep presenting a stale OFF as live.
 }finally{
  clearTimeout(timeout);
  statusBusy=false;
  refreshTelemetryAge();
 }
}
async function history(){
 if(trialActive)return;
 try { const r=await fetch('/api/history.csv',{cache:'no-store'});if(!r.ok)return;
 const str=await r.text(),lines=str.trim().split('\n');
 $('rows').textContent=lines.slice(-14).join('\n');
 let rows=lines.slice(1).map(x=>x.split(',')).filter(x=>x.length>=10&&x[4]==='1');
 let c=$('trend'),g=c.getContext('2d');
 g.clearRect(0,0,c.width,c.height);
 if(rows.length<2)return;
 let vals=rows.map(x=>Number(x[1]));
 let min=Math.floor(Math.min(...vals)-1),max=Math.ceil(Math.max(...vals)+1);
 if(max<=min)max=min+1;
 g.strokeStyle='#d5dce3';g.beginPath();g.moveTo(35,12);g.lineTo(35,200);g.lineTo(830,200);g.stroke();
 g.fillStyle='#52626c';g.font='12px sans-serif';g.fillText(max+' °C',3,20);g.fillText(min+' °C',3,200);
 g.strokeStyle='#087b80';g.lineWidth=2;g.beginPath();
 vals.forEach((v,i)=>{let x=35+(795*i/(vals.length-1)),y=200-180*(v-min)/(max-min);if(!i)g.moveTo(x,y);else g.lineTo(x,y)});g.stroke();
 }catch(e){}
}
async function events(){try{let r=await fetch('/api/events',{cache:'no-store'});if(r.ok)$('events').textContent=await r.text();}catch(e){}}
$('fastdash').href=window.location.protocol+'//'+window.location.hostname+':8081/';
 status();history();events();
setInterval(()=>{status();refreshTelemetryAge()},1000);
setInterval(history,4000);setInterval(events,2000);
</script></html>)HTML";
  }
};
