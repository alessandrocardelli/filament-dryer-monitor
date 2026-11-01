#include "../../MaterialPresets.h"
#include <cassert>
#include <iostream>
using namespace MaterialPresets;
int main(){
  static_assert(suggestion(MaterialProfile::Manual).durationMinutes==0);
  static_assert(suggestion(MaterialProfile::PLA).airTargetC==45);
  static_assert(suggestion(MaterialProfile::PLA).durationMinutes==360);
  static_assert(suggestion(MaterialProfile::PLAPlus).airTargetC==50);
  static_assert(suggestion(MaterialProfile::PLAPlus).durationMinutes==480);
  static_assert(suggestion(MaterialProfile::PVB).airTargetC==45);
  static_assert(suggestion(MaterialProfile::PVB).durationMinutes==480);
  static_assert(fromStored(255)==MaterialProfile::Manual);
  static_assert(fromStored(0)==MaterialProfile::Manual);
  static_assert(next(MaterialProfile::Manual,1)==MaterialProfile::PLA);
  static_assert(next(MaterialProfile::Manual,-1)==MaterialProfile::PVB);
  static_assert(next(MaterialProfile::PVB,1)==MaterialProfile::Manual);
  for(int i=1;i<=3;i++){
    const auto x=suggestion(fromStored(i));
    assert(x.airTargetC>=40 && x.airTargetC<=55);
    assert(x.durationMinutes>=15 && x.durationMinutes<=720);
    assert(x.durationMinutes%15==0);
  }
  std::cout<<"PASS HOST: three bounded material shortcuts and legacy MANUALE fallback. No GPIO or heating.\n";
}
