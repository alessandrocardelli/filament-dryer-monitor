#include "../../NtcQualification.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace NtcQualification;
static bool near(float a,float b,float tol=0.5f){return std::fabs(a-b)<tol;}
int main(){
  assert(near(expectedResistanceOhms(25,kAlternative),100000));
  assert(near(expectedResistanceOhms(25,kDonor),82500));
  assert(near(expectedResistanceOhms(20,kAlternative),125352,50));
  assert(near(expectedResistanceOhms(20,kDonor),104310,50));
  assert(near(estimatedBeadC(100000,kAlternative),25,0.01f));
  assert(near(estimatedBeadC(82500,kDonor),25,0.01f));
  for(int c=15;c<=65;c+=5){
    for(const Hypothesis &h:{kAlternative,kDonor}){
      const float r=expectedResistanceOhms((float)c,h);
      assert(std::isfinite(r) && r>0);
      assert(near(estimatedBeadC(r,h),c,0.001f));
      assert(near(differencePercent(r,c,h),0,0.001f));
    }
  }
  assert(std::isnan(expectedResistanceOhms(NAN,kAlternative)));
  assert(std::isnan(expectedResistanceOhms(-100,kAlternative)));
  assert(std::isnan(estimatedBeadC(0,kAlternative)));
  assert(std::isnan(estimatedBeadC(NAN,kDonor)));
  assert(std::isnan(differencePercent(-1,20,kDonor)));
  // At 20 C old 82.5k/B4100 predicts substantially lower R.
  assert(differencePercent(122300,20,kAlternative)<0.0f);
  assert(differencePercent(122300,20,kDonor)>0.0f);
  std::cout<<"PASS: NTC hypotheses diagnostic maths (no heater actuation)\n";
}
