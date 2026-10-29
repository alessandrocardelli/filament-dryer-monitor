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
#include "HeaterStep07GuardTrial.h"

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
    return R"HTML(<!doctype html><html lang="it"><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Filament Dryer</title>
<style>
:root{font-family:system-ui,Arial;color:#17202a;background:#f3f5f7}
body{max-width:960px;margin:20px auto;padding:0 16px}
header{display:flex;justify-content:space-between;align-items:center;gap:10px}
h1{font-size:1.6rem}a{color:#146b85}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:12px}
.card{padding:18px;border-radius:12px;background:white;border:1px solid #e2e6ea}
.value{font-size:1.9rem;font-weight:700}
small,.sub{color:#5d6771}
pre{white-space:pre-wrap;max-height:340px;overflow:auto;font-size:12px}
button{padding:9px 14px}
</style>
<header><h1>Filament Dryer Monitor</h1><small id="net">Connessione...</small></header>
<div class="grid">
<div class="card">Temperatura camera (SHT45)<div class="value" id="air">--</div></div>
<div class="card">Umidità camera (SHT45)<div class="value" id="rh">--</div></div>
<div class="card">Resistenza piastra (NTC)<div class="value" id="ntc">--</div></div>
<div class="card">Riscaldatore<div class="value" id="heat">--</div></div>
<div class="card">Ventola<div class="value" id="fan">--</div></div>
<div class="card">Test<div class="value" id="phase">--</div></div>
</div>
<p class="sub">SHT45: temperatura e umidità dell'aria nella camera. NTC: resistenza del sensore incorporato nel riscaldatore; temperatura locale non ancora validata. Non confrontare direttamente le due temperature.</p>
<p class="sub">Durante gli impulsi HEATER ON il firmware sospende le richieste HTTP per evitare ritardi del controllo: questa pagina NON visualizza in tempo reale l'accensione. Se mancano aggiornamenti, lo stato è NON AGGIORNATO (non OFF). Consultare il CSV a prova conclusa per la sequenza ON/OFF.</p>
<p id="ver" class="sub"></p>
<div class="card"><h2>Misure live (CSV, memoria RAM)</h2>
<p><a href="/api/history.csv">Scarica campioni live</a> ·
<a href="/api/last-test.csv">Scarica ultimo test salvato</a> ·
<a href="/ota">Aggiorna firmware OTA</a></p>
<canvas id="trend" width="850" height="220" style="width:100%;max-width:850px"></canvas>
<pre id="rows">...</pre>
<h3>Console eventi</h3><pre id="events">...</pre></div>
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
 const timeout=setTimeout(()=>controller.abort(),1800);
 try{
  const r=await fetch('/api/status',{cache:'no-store',signal:controller.signal});
  if(!r.ok)throw Error(r.status);
  const d=await r.json();
  trialActive=Boolean(d.cycle)||Number(d.fan_percent)>0;
  $('air').textContent=d.air_valid?d.air_c.toFixed(1)+' °C':'--';
  $('rh').textContent=d.air_valid?d.rh.toFixed(1)+' %':'--';
  $('ntc').textContent=d.ntc_valid?(d.ntc_ohms/1000).toFixed(2)+' kΩ':'--';
  $('heat').textContent=d.heater_command?'ON':'OFF';
  $('fan').textContent=d.fan_percent+' %';
  const waitNames=['','Minimum OFF 12 s','NTC attende 60 kΩ','Camera attende 24 °C'];
  $('phase').textContent=Number(d.phase)===3 && d.wait_reason
    ? 'HEATER OFF · '+(waitNames[Number(d.wait_reason)]||'Attesa controllo')
    :phaseLabel(d);
  $('ver').textContent='Firmware '+d.firmware+' | campioni '+d.ram_samples+
    (d.fault?' | FAULT '+d.pulse_fault:'')+
    ' | ON '+((d.cumulative_heat_ms||0)/1000).toFixed(1)+' s'+
    ' | CSV '+(d.session_csv_final?'COMPLETO':d.session_csv_saved?'CHECKPOINT':'non salvato in questa sessione')+
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
 g.strokeStyle='#08769a';g.lineWidth=2;g.beginPath();
 vals.forEach((v,i)=>{let x=35+(795*i/(vals.length-1)),y=200-180*(v-min)/(max-min);if(!i)g.moveTo(x,y);else g.lineTo(x,y)});g.stroke();
 }catch(e){}
}
async function events(){try{let r=await fetch('/api/events',{cache:'no-store'});if(r.ok)$('events').textContent=await r.text();}catch(e){}}
status();history();events();
setInterval(()=>{status();refreshTelemetryAge()},1000);
setInterval(history,4000);setInterval(events,2000);
</script></html>)HTML";
  }
};
