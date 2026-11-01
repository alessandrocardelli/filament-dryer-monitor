// HOST ONLY: validate actual inline :8081 dashboard script, command UI and
// same-origin POST design. No ESP32 or active heater.
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const src=fs.readFileSync('firmware/LiveTelemetryServer.cpp','utf8');
const match=src.match(/static const char kLivePage\[\] = R"HTML\(([\s\S]*?)\)HTML";/);
assert.ok(match,'No actual dashboard markup found');
const html=match[1];
const script=html.match(/<script>([\s\S]*?)<\/script>/);
assert.ok(script,'Inline script missing');
new vm.Script(script[1],{filename:'remote-dashboard.js'});
const els={};
const get=id=>els[id]||(els[id]={value:'',textContent:'',disabled:false,handlers:{},
  addEventListener:function(type,fn){this.handlers[type]=fn;}});
const ctx=vm.createContext({
 document:{getElementById:get},
 TextEncoder, btoa,AbortController:class {constructor(){this.signal={}}abort(){}},
 setTimeout:()=>1,clearTimeout:()=>{},setInterval:()=>{},
 Date:{now:()=>1000},
 confirm:()=>false,
 fetch:()=>Promise.reject(Error('offline'))
});
vm.runInContext(script[1],ctx);
get('profile').value='2';get('profile').handlers.change();
assert.equal(get('temp').value,50);
assert.equal(get('duration').value,480);
get('temp').value='51';get('temp').handlers.input();
assert.equal(get('profile').value,'0'); // manual override
assert.match(src,/stop\.uri="\/api\/stop"/);
assert.match(src,/set\.uri="\/api\/settings"/);
assert.doesNotMatch(src,/\.uri="\/api\/start"/);
assert.match(src,/Authorization/);
assert.match(src,/X-Dryer-Command/);
assert.match(src,/WiFi\.status\(\)!=WL_CONNECTED/);
assert.match(src,/!p->settingsPending && !p->stopPending/);
assert.match(src,/!p->authEnabled|!enabled/);
assert.match(src,/httpd_resp_set_status\(req,"409 Conflict"\)/);
assert.match(src,/RemoteControlPolicy::valid\(command\)/);
assert.match(src,/RemoteControlPolicy::canStop/);
assert.match(src,/p->stopPending=true/);
console.log('PASS authenticated LAN settings and STOP UI; no remote START, manual override, same-origin controls');
