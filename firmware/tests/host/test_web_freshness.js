// Run on Linux Actions: node firmware/tests/host/test_web_freshness.js
// Isolate inline dashboard JS from the actual NetworkService.h source.
// No ESP32, GPIO, physical heat or live network.
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const header=fs.readFileSync('firmware/NetworkService.h','utf8');
const script=header.match(/<script>([\s\S]*?)<\/script>/);
assert.ok(script, 'dashboard inline script must exist');
const text=script[1];
new vm.Script(text, {filename:'dashboard-inline.js'}); // syntax validation
const el={};
const element=id=>el[id]||(el[id]={textContent:'OFF',getContext:()=>({})});
const fakeNow={value:5000};
const ctx=vm.createContext({
  document:{getElementById:element},
  Date:{now:()=>fakeNow.value},
  setTimeout:()=>1,
  clearTimeout:()=>{},
  setInterval:()=>{},
  AbortController:class {constructor(){this.signal={}}abort(){}},
  fetch:()=>Promise.reject(new Error('offline'))
});
// Initialize only the functions, not the auto-calling polling at the end.
const noAuto=text.replace(/status\(\);history\(\);events\(\);[\s\S]*$/, '');
vm.runInContext(noAuto,ctx);
function label(data){return vm.runInContext('phaseLabel('+JSON.stringify(data)+')',ctx)}
assert.equal(label({phase:4,pulses:4,pulse_goal:4}),'Raffreddamento');
assert.equal(label({phase:2,pulses:3,pulse_goal:4}),'Riscaldamento · impulso 4 di 4');
assert.equal(label({phase:3,pulses:2,pulse_goal:4}),'Pausa dopo impulso 2 di 4');
assert.equal(label({phase:5,pulses:4,pulse_goal:4}),'Test completato');
assert.equal(label({phase:6,pulses:4,pulse_goal:4}),'Test interrotto');
vm.runInContext('statusReceivedAt=1000;refreshTelemetryAge()',ctx);
assert.equal(element('heat').textContent,'NON AGGIORNATO');
assert.match(element('net').textContent,/Telemetria sospesa/);
fakeNow.value=1100;
element('heat').textContent='OFF';
vm.runInContext('refreshTelemetryAge()',ctx);
assert.equal(element('heat').textContent,'OFF'); // fresh response still allowed
assert.match(header,/server_\.sendHeader\("Cache-Control", "no-store, no-cache, must-revalidate"\)/);
assert.match(header,/if \(\(permitHttp \|\| uploadActive_\) && !ideOtaBusy_\)/);
console.log('PASS: web source JS parses; phase labels, stale OFF and HTML no-cache verified');
