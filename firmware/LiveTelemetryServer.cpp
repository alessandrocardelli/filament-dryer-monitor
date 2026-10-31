#include "LiveTelemetryServer.h"
#include <Arduino.h>
#include <esp_http_server.h>
#include <cmath>
#include <cstdio>
#include "FirmwareBuild.h"

// Same lock for publish/read; critical section contains ONLY a small struct
// copy (no network, no String or allocation, no GPIO, no filesystem).
struct LiveTelemetryServer::Internal {
  portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
  Snapshot snapshot{};
  httpd_handle_t handle=nullptr;
};

static const char kLivePage[] = R"HTML(<!doctype html><html lang="it"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Filament Dryer - dati live</title>
<style>body{font-family:system-ui;margin:1.5rem auto;padding:0 1rem;max-width:650px;color:#223;background:#f5f7f8}h1{font-size:1.5rem}.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:12px}.card{padding:16px;background:white;border-radius:10px}.value{font-size:1.8rem;font-weight:700}small{color:#52606d}</style>
<h1>Filament Dryer - dati in tempo reale</h1><p id="age">Connessione...</p>
<div class="grid"><div class="card">Aria<div class="value" id="air">--</div></div><div class="card">Umidita<div class="value" id="rh">--</div></div><div class="card">NTC piastra<div class="value" id="ntc">--</div></div><div class="card">Riscaldatore<div class="value" id="heat">--</div></div><div class="card">Ventola<div class="value" id="fan">--</div></div><div class="card">Ciclo<div class="value" id="cycle">--</div></div></div>
<p id="notes"><small>Il sito principale (porta 80) offre grafici, CSV e OTA. La NTC indica la resistenza, non la temperatura del termistore.</small></p>
<script>
let last=0;const el=id=>document.getElementById(id);
async function refresh(){
 const c=new AbortController(),t=setTimeout(()=>c.abort(),1400);
 try {const r=await fetch('/api/status',{signal:c.signal,cache:'no-store'});if(!r.ok)throw Error(r.status);
 const d=await r.json();
 el('air').textContent=d.air_valid?d.air_c.toFixed(1)+' °C':'--';
 el('rh').textContent=d.air_valid?d.rh.toFixed(1)+' %':'--';
 el('ntc').textContent=d.ntc_valid?(d.ntc_ohms/1000).toFixed(2)+' kΩ':'--';
 el('heat').textContent=d.heater_command?'ON':'OFF';
 el('fan').textContent=d.fan_percent+'%';
 el('cycle').textContent=d.fault?'FAULT':d.cycle?'IN CORSO':'FERMO';
 el('notes').textContent='Firmware '+d.firmware+' · '+d.ram_samples+' campioni'+(d.log_truncated?' · LOG PARZIALE':'')+' · NTC: resistenza misurata, non temperatura esatta.';
 last=Date.now();el('age').textContent='Dati aggiornati';
 }catch(e){}finally{clearTimeout(t);if(Date.now()-last>2500){el('heat').textContent='NON AGGIORNATO';el('age').textContent='Collegamento non aggiornato';}}
}
refresh();setInterval(refresh,1000);
</script></html>)HTML";

void LiveTelemetryServer::begin() {
  if(internal_)return;
  internal_=new Internal{};
  if(!internal_)return;
  httpd_config_t config=HTTPD_DEFAULT_CONFIG();
  config.server_port=8081;
  config.ctrl_port=32769; // must differ from HTTP server default
  config.max_uri_handlers=2;
  config.stack_size=4096;
  config.lru_purge_enable=true;
  config.recv_wait_timeout=1;
  config.send_wait_timeout=1;
  if(httpd_start(&internal_->handle,&config)!=ESP_OK) {
    delete internal_;internal_=nullptr;
    Serial.println("LIVE HTTP 8081 start failed; safety control unaffected");
    return;
  }
  httpd_uri_t page{};
  page.uri="/";page.method=HTTP_GET;page.handler=[](httpd_req_t *request)->esp_err_t{
    httpd_resp_set_type(request,"text/html; charset=utf-8");
    httpd_resp_set_hdr(request,"Cache-Control","no-store");
    return httpd_resp_send(request,kLivePage,HTTPD_RESP_USE_STRLEN);
  };
  httpd_uri_t api{};
  api.uri="/api/status";api.method=HTTP_GET;api.user_ctx=this;
  api.handler=[](httpd_req_t *request)->esp_err_t{
    auto *self=static_cast<LiveTelemetryServer *>(request->user_ctx);
    Snapshot s{};
    if(self->internal_) {
      portENTER_CRITICAL(&self->internal_->mux);
      s=self->internal_->snapshot;
      portEXIT_CRITICAL(&self->internal_->mux);
    }
    // Only bounded stack formatting. No shared network, FS or NTC control.
    char body[740];
    const int n=snprintf(body,sizeof(body),
      "{\"firmware\":\"%s\",\"ms\":%lu,\"air_c\":%.2f,\"rh\":%.2f,"
      "\"ntc_ohms\":%.1f,\"air_valid\":%s,\"ntc_valid\":%s,"
      "\"heater_command\":%s,\"cycle\":%s,\"fault\":%s,"
      "\"fan_percent\":%u,\"phase\":%u,\"pulses\":%u,\"pulse_goal\":0,"
      "\"pulse_fault\":%u,\"wait_reason\":%u,\"cumulative_heat_ms\":%lu,"
      "\"session_csv_saved\":%s,\"session_csv_final\":%s,\"saved_log\":%s,"
      "\"ram_samples\":%u,\"log_truncated\":%s,"
      "\"session_ntc_min_ohms\":%.1f,\"ota_idle\":false}",
      FirmwareBuild::kId,(unsigned long)s.ms,
      s.airValid?s.airC:0.0f,s.airValid?s.rh:0.0f,s.ntcValid?s.ntcOhms:0.0f,
      s.airValid?"true":"false",s.ntcValid?"true":"false",
      s.heaterCommand?"true":"false",s.cycle?"true":"false",s.fault?"true":"false",
      (unsigned)s.fanPercent,(unsigned)s.phase,(unsigned)s.pulses,
      (unsigned)s.pulseFault,(unsigned)s.waitReason,
      (unsigned long)s.cumulativeHeatMs,
      s.sessionCsvSaved?"true":"false",s.sessionCsvFinal?"true":"false",
      s.sessionCsvSaved?"true":"false",(unsigned)s.ramSamples,
      s.logTruncated?"true":"false",
      std::isfinite(s.minSessionNtcOhms)?s.minSessionNtcOhms:0.0f);
    if(n<=0||n>=(int)sizeof(body)) {
      httpd_resp_send_err(request,HTTPD_500_INTERNAL_SERVER_ERROR,"status unavailable");
      return ESP_FAIL;
    }
    httpd_resp_set_type(request,"application/json");
    httpd_resp_set_hdr(request,"Cache-Control","no-store");
    httpd_resp_set_hdr(request,"Access-Control-Allow-Origin","*"); // read-only LAN telemetry
    return httpd_resp_send(request,body,n);
  };
  if(httpd_register_uri_handler(internal_->handle,&page)!=ESP_OK ||
     httpd_register_uri_handler(internal_->handle,&api)!=ESP_OK) {
    httpd_stop(internal_->handle);delete internal_;internal_=nullptr;
    Serial.println("LIVE HTTP routes failed; safety control unaffected");
    return;
  }
  Serial.println("Read-only live telemetry: http://<device-ip>:8081/");
}

void LiveTelemetryServer::publish(const Snapshot &s) {
  if(!internal_)return;
  portENTER_CRITICAL(&internal_->mux);
  internal_->snapshot=s;
  portEXIT_CRITICAL(&internal_->mux);
}
