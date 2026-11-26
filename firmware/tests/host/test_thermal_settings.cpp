#include "../../ThermalSettings.h"
#include <cassert>
#include <iostream>
int main(){
  using namespace ThermalSettings;
  static_assert(!validForTarget(Values{},55));
  Values s{};assert(!validForTarget(s,55));
  assert(adjust(0,+1,20,150)==20);
  assert(adjust(20,-1,20,150)==0);
  assert(adjust(150,+1,20,150)==150);
  s.ntcRearmC=75;s.ntcPauseC=85;s.ntcFaultC=100;s.airFaultC=60;
  assert(validForTarget(s,55));
  auto k=s;k.ntcRearmC=85;assert(!validForTarget(k,55));
  k=s;k.ntcFaultC=85;assert(!validForTarget(k,55));
  k=s;k.airFaultC=55;assert(!validForTarget(k,55));
  k=s;k.ntcPauseC=151;sanitize(k);
  assert(k.ntcPauseC==0 && !validForTarget(k,55));
  assert(equal(s,s)&&!equal(s,k));
  std::cout<<"PASS four separate editable thresholds, NVS sanitization and ordering\n";
}
