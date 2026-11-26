#include "../../VishayNtcCurve.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
  using namespace VishayNtcCurve;
  static_assert(sizeof(kOhms)/sizeof(kOhms[0])==39);
  assert(std::isnan(temperatureC(NAN)));
  assert(std::isnan(temperatureC(0.0f)));
  assert(std::isnan(temperatureC(kOhms[0]+100.0f)));
  assert(std::isnan(temperatureC(kOhms[38]-1.0f)));
  for(int i=0;i<39;++i){
    assert(std::fabs(temperatureC(kOhms[i])-(-40.0f+5.0f*i))<0.03f);
    if(i>0)assert(kOhms[i]<kOhms[i-1]);
  }
  assert(std::fabs(temperatureC(100000.0f)-25.0f)<0.01f);
  assert(std::fabs(temperatureC(9988.4f)-85.0f)<0.03f);
  assert(std::fabs(temperatureC(8504.3f)-90.0f)<0.03f);
  assert(std::fabs(temperatureC(8200.0f)-91.2f)<0.2f);
  for(int i=0;i<38;++i){
    const float r=std::sqrt(kOhms[i]*kOhms[i+1]);
    assert(std::fabs(temperatureC(r)-(-37.5f+5.0f*i))<0.03f);
  }
  std::cout<<"PASS Vishay exact manufacturer anchors, log-R interpolation, bounds\n";
}
