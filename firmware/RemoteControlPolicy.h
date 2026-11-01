#pragma once
#include <cstdint>

// The network task only queues commands. Application loop revalidates state.
// No network thread ever writes heater GPIO, NVS, or starts a cycle.
// Settings are allowed only in idle Standby without postheat cooling.
// Remote STOP affects only Drying, never clears a latched FAULT.
namespace RemoteControlPolicy {
enum class State : uint8_t { Boot=0,Standby=1,Setup=2,Settings=3,Drying=4,Complete=5,Fault=6 };
struct Settings {uint8_t profile=0;uint16_t tempC=0,durationMinutes=0;};
constexpr bool canEdit(State state,bool cooling,bool updateBusy,bool heaterOn) {
  return state==State::Standby && !cooling && !updateBusy && !heaterOn;
}
constexpr bool canStop(State state) { return state==State::Drying; }
constexpr bool valid(Settings s){
  if(s.profile>3 || s.tempC<40 || s.tempC>55 ||
     s.durationMinutes<15 || s.durationMinutes>720 ||
     s.durationMinutes%15!=0) return false;
  // A named material shortcut must correspond to its exact published
  // suggestion; editing either field returns the label to MANUALE.
  switch(s.profile){
    case 1: return s.tempC==45 && s.durationMinutes==360; // PLA
    case 2: return s.tempC==50 && s.durationMinutes==480; // eSUN PLA+
    case 3: return s.tempC==45 && s.durationMinutes==480; // PVB
    default:return true;
  }
}
} // namespace RemoteControlPolicy
