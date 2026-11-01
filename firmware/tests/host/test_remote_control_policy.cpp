#include "../../RemoteControlPolicy.h"
#include <cassert>
#include <iostream>
using namespace RemoteControlPolicy;
int main(){
  static_assert(canEdit(State::Standby,false,false,false));
  static_assert(!canEdit(State::Drying,false,false,false));
  static_assert(!canEdit(State::Standby,true,false,false));
  static_assert(!canEdit(State::Standby,false,true,false));
  static_assert(!canEdit(State::Standby,false,false,true));
  static_assert(!canEdit(State::Fault,false,false,false));
  static_assert(canStop(State::Drying));
  static_assert(!canStop(State::Standby));
  static_assert(!canStop(State::Fault));
  static_assert(valid({0,40,15})&&valid({0,55,720}));
  static_assert(valid({1,45,360})&&valid({2,50,480})&&valid({3,45,480}));
  static_assert(!valid({3,50,480})); // changed preset must become Manual
  static_assert(!valid({1,45,480}));
  static_assert(!valid({4,45,480}));
  static_assert(!valid({0,39,60})&&!valid({0,56,60}));
  static_assert(!valid({0,40,0})&&!valid({0,40,721}));
  static_assert(!valid({0,40,61}));
  std::cout<<"PASS remote control boundaries: only Standby settings, "
              "Drying-only STOP, no remote START, strict UI-safe values.\n";
}
