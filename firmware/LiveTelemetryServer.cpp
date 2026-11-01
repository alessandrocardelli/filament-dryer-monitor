#include "LiveTelemetryServer.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_http_server.h>
#include <mbedtls/base64.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "FirmwareBuild.h"

// ESP-IDF owns HTTP service on core/task independent of safety loop. Only
// fixed-size snapshots and queued requests cross the FreeRTOS spinlock.
// All NVS, GPIO, STOP/cooldown state changes are applied by Arduino loop().
struct LiveTelemetryServer::Internal {
  portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
  Snapshot snapshot{};
  httpd_handle_t handle=nullptr;
  char basicAuth[132]{};
  bool authEnabled=false;
  bool stopPending=false, settingsPending=false;
  RemoteControlPolicy::Settings requested{};
};

static const char kLivePage[] = R"HTML(<!doctype html><html lang="it">
<head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Filament Dryer - Controllo</title>
<style>
body{font-family:system-ui;margin:1.2rem auto;padding:0 1rem;max-width:690px;color:#223;background:#f5f7f8}
h1{font-size:1.5rem}.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}
.card{padding:16px;background:white;border-radius:10px}.value{font-size:1.55rem;font-weight:700}
small{color:#52606d}label{display:block;margin:.8rem 0}input,select{padding:.55rem;font-size:1rem;max-width:100%;box-sizing:border-box}
button{padding:.65rem 1rem;font-size:1rem;margin:.5rem .4rem .5rem 0;border:0;border-radius:6px;background:#22516e;color:white}
button:disabled{opacity:.5}#stop{background:#a02121}#result{min-height:1.5rem}
</style></head><body>
<h1>Filament Dryer</h1><p id="age">Connessione...</p>
<div class="grid">
<div class="card">Aria<div class="value" id="air">--</div></div>
<div class="card">Umidita<div class="value" id="rh">--</div></div>
<div class="card">NTC piastra<div class="value" id="ntc">--</div></div>
<div class="card">Riscaldatore<div class="value" id="heat">--</div></div>
<div class="card">Ventola<div class="value" id="fan">--</div></div>
<div class="card">Ciclo<div class="value" id="cycle">--</div></div>
</div>
<div class="card" style="margin-top:12px">
<h2>Comandi in rete locale</h2>
<small>Le impostazioni possono cambiare solo in STANDBY, senza raffreddamento.
STOP è disponibile durante un ciclo. Nessun avvio remoto.</small>
<label>Password amministratore (quella gia usata per OTA)
<input id="password" type="password" autocomplete="off" maxlength="63"></label>
<label>Profilo
<select id="profile"><option value="0">Manuale</option><option value="1">PLA</option><option value="2">PLA+</option><option value="3">PVB</option></select></label>
<label>Temperatura aria (40-55 C)
<input id="temp" type="number" min="40" max="55" step="1"></label>
<label>Durata (minuti, multipli di 15, massimo 720)
<input id="duration" type="number" min="15" max="720" step="15"></label>
<button type="button" id="save" disabled>Salva impostazioni</button>
<button type="button" id="stop" disabled>STOP ciclo</button>
<p id="result"></p>
<small>I comandi sono inviati tramite HTTP solo nella rete Wi-Fi fidata.
Non aprire porte del router verso Internet. Per fuori casa usare VPN.
La password non viene memorizzata nel browser.</small>
</div><p><a href="http://filament-dryer.local/">Dashboard storica, CSV e OTA</a></p>
<script>
'use strict';
const el=id=>document.getElementById(id);
let last=0,editing=false,syncPending=false,busy=false;
const options=[[0,0],[45,360],[50,480],[45,480]];
function userEdited(){editing=true;}
el('profile').addEventListener('change',()=>{
 const p=Number(el('profile').value);
 if(p>0){el('temp').value=options[p][0];el('duration').value=options[p][1];}
 userEdited();
});
for(const id of ['temp','duration'])el(id).addEventListener('input',()=>{
 el('profile').value='0';userEdited();
});
function setInputs(d){
 if(!editing && !syncPending){
   el('profile').value=String(d.profile);
   el('temp').value=String(d.target_c);
   el('duration').value=String(d.duration_min);
 }
}
function remoteAuth(){
 const secret=el('password').value;
 if(!secret)throw Error('Inserisci la password amministratore');
 const b=new TextEncoder().encode('admin:'+secret);
 let raw='';for(const n of b)raw+=String.fromCharCode(n);
 return 'Basic '+btoa(raw);
}
async function command(path,body){
 if(busy)return;
 busy=true;el('result').textContent='Invio comando...';
 try{
  const authorization=remoteAuth();
  const ctrl=new AbortController(),timeout=setTimeout(()=>ctrl.abort(),2000);
  let res;
  try {res=await fetch(path,{method:'POST',cache:'no-store',signal:ctrl.signal,
   headers:{'Authorization':authorization,'X-Dryer-Command':'1',
            'Content-Type':'text/plain;charset=UTF-8'},body:body||''});}
  finally{clearTimeout(timeout);}
  const message=await res.text();
  if(!res.ok)throw Error('HTTP '+res.status+': '+message);
  if(path==='/api/settings'){editing=false;syncPending=true;}
  el('result').textContent=message;
 }catch(e){el('result').textContent=e.message;}
 finally{busy=false;}
}
el('save').addEventListener('click',()=>{
 const p=Number(el('profile').value),t=Number(el('temp').value),m=Number(el('duration').value);
 if(!Number.isInteger(t)||t<40||t>55||!Number.isInteger(m)||
    m<15||m>720||m%15!==0){el('result').textContent='Temperatura/durata non valide';return;}
 command('/api/settings',p+','+t+','+m);
});
el('stop').addEventListener('click',()=>{
 if(confirm('Interrompere il ciclo in corso?'))command('/api/stop','');
});
async function refresh(){
 const c=new AbortController(),timer=setTimeout(()=>c.abort(),1400);
 try{
  const r=await fetch('/api/status',{signal:c.signal,cache:'no-store'});
  if(!r.ok)throw Error('HTTP '+r.status);
  const d=await r.json();
  el('air').textContent=d.air_valid?d.air_c.toFixed(1)+' °C':'--';
  el('rh').textContent=d.air_valid?d.rh.toFixed(1)+' %':'--';
  el('ntc').textContent=d.ntc_valid?(d.ntc_ohms/1000).toFixed(2)+' kΩ':'--';
  el('heat').textContent=d.heater_command?'ON':'OFF';
  el('fan').textContent=d.fan_percent+'%';
  el('cycle').textContent=d.fault?'FAULT':d.cycle?'IN CORSO':'FERMO';
  el('notes');
  if(syncPending){syncPending=false;editing=false;}
  setInputs(d);
  el('save').disabled=!d.can_edit;
  el('stop').disabled=!d.can_stop;
  last=Date.now();el('age').textContent='Dati aggiornati · '+d.firmware+
     (d.log_truncated?' · LOG PARZIALE':'');
 }catch(e){}finally{
  clearTimeout(timer);
  if(Date.now()-last>2500){
   el('age').textContent='Collegamento NON AGGIORNATO';
   el('heat').textContent='NON AGGIORNATO';
   el('save').disabled=true;el('stop').disabled=true;
  }
 }
}
refresh();setInterval(refresh,1000);
</script></body></html>)HTML";

// Require Basic admin authentication AND a non-simple custom request header:
// a foreign origin cannot submit these POSTs with a browser without CORS
// preflight, and this service grants NO CORS rights for control routes.
// HTTP Basic is NOT encrypted; use only a trusted local Wi-Fi, never WAN.
static bool commandsAuthorized(httpd_req_t *req,const char *expected,bool enabled){
  if(!enabled || WiFi.status()!=WL_CONNECTED)return false;
  char marker[8]{};
  if(httpd_req_get_hdr_value_str(req,"X-Dryer-Command",
                                 marker,sizeof(marker))!=ESP_OK ||
     strcmp(marker,"1")!=0) return false;
  char authorization[132]{};
  if(httpd_req_get_hdr_value_str(req,"Authorization",
                                 authorization,sizeof(authorization))!=ESP_OK)
    return false;
  const size_t actual=strlen(authorization),correct=strlen(expected);
  unsigned diff=static_cast<unsigned>(actual^correct);
  const size_t size=actual>correct?actual:correct;
  for(size_t i=0;i<size&&i<sizeof(authorization);++i)
    diff|=static_cast<unsigned>(authorization[i]^expected[i]);
  return diff==0;
}
static esp_err_t conflict(httpd_req_t *req,const char *msg){
  httpd_resp_set_status(req,"409 Conflict");
  httpd_resp_set_hdr(req,"Cache-Control","no-store");
  return httpd_resp_sendstr(req,msg);
}
static esp_err_t respond(httpd_req_t *req,httpd_err_code_t code,const char *msg){
  httpd_resp_set_hdr(req,"Cache-Control","no-store");
  return httpd_resp_send_err(req,code,msg);
}

void LiveTelemetryServer::begin(const char *adminPassword) {
  if(internal_)return;
  internal_=new Internal{};
  if(!internal_)return;
  // No command routes are usable unless provisioning set a >=12-char admin.
  // Keep the expected Basic token in RAM and never print/store credentials.
  const size_t len=adminPassword?strlen(adminPassword):0;
  if(len>=12 && len<=63) {
    char plain[71]{};
    const int n=snprintf(plain,sizeof(plain),"admin:%s",adminPassword);
    size_t encoded=0;
    unsigned char base64[104]{};
    if(n>0 && n<(int)sizeof(plain) &&
       mbedtls_base64_encode(base64,sizeof(base64),&encoded,
                             reinterpret_cast<const unsigned char *>(plain),
                             static_cast<size_t>(n))==0 &&
       encoded+7<sizeof(internal_->basicAuth)){
      memcpy(internal_->basicAuth,"Basic ",6);
      memcpy(internal_->basicAuth+6,base64,encoded);
      internal_->basicAuth[encoded+6]='\0';
      internal_->authEnabled=true;
    }
    memset(plain,0,sizeof(plain));
  }
  httpd_config_t config=HTTPD_DEFAULT_CONFIG();
  config.server_port=8081;
  config.ctrl_port=32769; // different from port80 server
  config.max_uri_handlers=4; // page/status/settings/stop, no start route
  config.stack_size=6144;
  config.lru_purge_enable=true;
  config.recv_wait_timeout=1;
  config.send_wait_timeout=1;
  if(httpd_start(&internal_->handle,&config)!=ESP_OK){
    delete internal_;internal_=nullptr;
    Serial.println("LIVE HTTP 8081 start failed; safety unaffected");
    return;
  }
  httpd_uri_t page{};
  page.uri="/";page.method=HTTP_GET;
  page.handler=[](httpd_req_t *request)->esp_err_t{
    httpd_resp_set_type(request,"text/html; charset=utf-8");
    httpd_resp_set_hdr(request,"Cache-Control","no-store");
    return httpd_resp_send(request,kLivePage,HTTPD_RESP_USE_STRLEN);
  };
  httpd_uri_t api{};
  api.uri="/api/status";api.method=HTTP_GET;api.user_ctx=this;
  api.handler=[](httpd_req_t *request)->esp_err_t{
    auto *self=static_cast<LiveTelemetryServer *>(request->user_ctx);
    Snapshot s{};
    if(self->internal_){
      portENTER_CRITICAL(&self->internal_->mux);
      s=self->internal_->snapshot;
      portEXIT_CRITICAL(&self->internal_->mux);
    }
    char body[880];
    const int n=snprintf(body,sizeof(body),
      "{\"firmware\":\"%s\",\"ms\":%lu,\"air_c\":%.2f,\"rh\":%.2f,"
      "\"ntc_ohms\":%.1f,\"air_valid\":%s,\"ntc_valid\":%s,"
      "\"heater_command\":%s,\"cycle\":%s,\"fault\":%s,"
      "\"fan_percent\":%u,\"phase\":%u,\"pulses\":%u,\"pulse_goal\":0,"
      "\"pulse_fault\":%u,\"wait_reason\":%u,\"cumulative_heat_ms\":%lu,"
      "\"session_csv_saved\":%s,\"session_csv_final\":%s,\"saved_log\":%s,"
      "\"ram_samples\":%u,\"log_truncated\":%s,"
      "\"session_ntc_min_ohms\":%.1f,\"ota_idle\":false,"
      "\"app_state\":%u,\"profile\":%u,\"target_c\":%u,"
      "\"duration_min\":%u,\"can_edit\":%s,\"can_stop\":%s}",
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
      std::isfinite(s.minSessionNtcOhms)?s.minSessionNtcOhms:0.0f,
      (unsigned)s.appState,(unsigned)s.profile,(unsigned)s.targetC,
      (unsigned)s.durationMinutes,s.canEdit?"true":"false",
      RemoteControlPolicy::canStop(
        static_cast<RemoteControlPolicy::State>(s.appState))?"true":"false");
    if(n<=0||n>=(int)sizeof(body))
      return respond(request,HTTPD_500_INTERNAL_SERVER_ERROR,"status overflow");
    httpd_resp_set_type(request,"application/json");
    httpd_resp_set_hdr(request,"Cache-Control","no-store");
    httpd_resp_set_hdr(request,"Access-Control-Allow-Origin","*"); // GET only
    return httpd_resp_send(request,body,n);
  };
  httpd_uri_t set{};
  set.uri="/api/settings";set.method=HTTP_POST;set.user_ctx=this;
  set.handler=[](httpd_req_t *request)->esp_err_t{
    auto *self=static_cast<LiveTelemetryServer *>(request->user_ctx);
    Internal *p=self->internal_;
    if(!p || !commandsAuthorized(request,p->basicAuth,p->authEnabled))
      return respond(request,HTTPD_401_UNAUTHORIZED,"Admin authorization required");
    if(request->content_len<=0 || request->content_len>=32)
      return respond(request,HTTPD_400_BAD_REQUEST,"Invalid input length");
    char body[32]{};
    const int n=httpd_req_recv(request,body,request->content_len);
    if(n<=0 || n!=request->content_len)
      return respond(request,HTTPD_400_BAD_REQUEST,"Invalid body");
    unsigned profile=0,temp=0,duration=0;char trailing=0;
    if(sscanf(body,"%u,%u,%u%c",&profile,&temp,&duration,&trailing)!=3)
      return respond(request,HTTPD_400_BAD_REQUEST,"Invalid values");
    RemoteControlPolicy::Settings command{
      static_cast<uint8_t>(profile),static_cast<uint16_t>(temp),
      static_cast<uint16_t>(duration)};
    char canonical[32]{};
    snprintf(canonical,sizeof(canonical),"%u,%u,%u",profile,temp,duration);
    if(strcmp(body,canonical)!=0 || profile>255 || temp>65535 ||
       duration>65535 || !RemoteControlPolicy::valid(command))
      return respond(request,HTTPD_400_BAD_REQUEST,"Out of allowed range");
    bool accepted=false;
    portENTER_CRITICAL(&p->mux);
    const auto st=static_cast<RemoteControlPolicy::State>(p->snapshot.appState);
    if(p->snapshot.canEdit && st==RemoteControlPolicy::State::Standby &&
       !p->settingsPending && !p->stopPending){
      p->requested=command;p->settingsPending=true;accepted=true;
    }
    portEXIT_CRITICAL(&p->mux);
    if(!accepted)return conflict(request,
                                "Only in idle STANDBY; no command queued");
    httpd_resp_set_type(request,"text/plain; charset=utf-8");
    return httpd_resp_sendstr(request,"Settings queued; verify new values on status");
  };
  httpd_uri_t stop{};
  stop.uri="/api/stop";stop.method=HTTP_POST;stop.user_ctx=this;
  stop.handler=[](httpd_req_t *request)->esp_err_t{
    auto *self=static_cast<LiveTelemetryServer *>(request->user_ctx);
    Internal *p=self->internal_;
    if(!p || !commandsAuthorized(request,p->basicAuth,p->authEnabled))
      return respond(request,HTTPD_401_UNAUTHORIZED,"Admin authorization required");
    if(request->content_len!=0)
      return respond(request,HTTPD_400_BAD_REQUEST,"STOP takes no payload");
    bool accepted=false;
    portENTER_CRITICAL(&p->mux);
    if(RemoteControlPolicy::canStop(
        static_cast<RemoteControlPolicy::State>(p->snapshot.appState))) {
      p->stopPending=true;accepted=true; // idempotent if repeated
      p->settingsPending=false; // STOP has priority over queued settings
    }
    portEXIT_CRITICAL(&p->mux);
    if(!accepted)return conflict(request,"No active cycle");
    httpd_resp_set_type(request,"text/plain; charset=utf-8");
    return httpd_resp_sendstr(request,"STOP queued; check HEATER OFF status");
  };
  if(httpd_register_uri_handler(internal_->handle,&page)!=ESP_OK ||
     httpd_register_uri_handler(internal_->handle,&api)!=ESP_OK ||
     httpd_register_uri_handler(internal_->handle,&set)!=ESP_OK ||
     httpd_register_uri_handler(internal_->handle,&stop)!=ESP_OK){
    httpd_stop(internal_->handle);delete internal_;internal_=nullptr;
    Serial.println("LIVE HTTP routes failed; safety control unaffected");
    return;
  }
  Serial.println("LAN dashboard & authenticated settings/STOP :8081 (NO START)");
}

void LiveTelemetryServer::publish(const Snapshot &s){
  if(!internal_)return;
  portENTER_CRITICAL(&internal_->mux);
  internal_->snapshot=s;
  portEXIT_CRITICAL(&internal_->mux);
}

bool LiveTelemetryServer::takeStopRequest(){
  if(!internal_)return false;
  portENTER_CRITICAL(&internal_->mux);
  const bool requested=internal_->stopPending;
  internal_->stopPending=false;
  if(requested)internal_->settingsPending=false;
  portEXIT_CRITICAL(&internal_->mux);
  return requested;
}

bool LiveTelemetryServer::takeSettingsRequest(RemoteControlPolicy::Settings &out){
  if(!internal_)return false;
  portENTER_CRITICAL(&internal_->mux);
  const bool found=internal_->settingsPending && !internal_->stopPending;
  if(found)out=internal_->requested;
  internal_->settingsPending=false;
  portEXIT_CRITICAL(&internal_->mux);
  return found;
}
