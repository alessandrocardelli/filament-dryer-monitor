#include <cassert>
#include <cmath>
#include <iostream>
#include "../../NtcReadoutCalibration.h"
#include "../../VishayNtcCurve.h"

// Original seven Sanwa DMM references and 21 HTTP readings from 26 Nov.
// Input mV is analytically reconstructed, NOT a measured ADC sample.
// In-sample replay, not an independent hardware validation.
struct Row { float dmm; float legacy[3]; };
constexpr Row data[] = {
  {99900, {100154.4f,100853.5f,100017.1f}},
  {46800, {47433.7f,47444.2f,47170.0f}},
  {21930, {22018.8f,21928.9f,22092.1f}},
  {9970, {10111.6f,10096.2f,10105.2f}},
  {8170, {8256.6f,8262.6f,8225.4f}},
  {6770, {6919.1f,6922.5f,6883.7f}},
  {4690, {4849.3f,4851.4f,4806.1f}}
};
constexpr float supply=3311.0f, pullup=46572.0f;
static float legacyMv(float r){return supply*r/(pullup+r);}
static float corrected(float v){return NtcReadoutCalibration::resistanceOhms(v,supply,pullup);}
int main(){
  float maxSample=0, maxMean=0, maxTemp=0;
  for(const auto& row:data){
    float sum=0;
    for(float old:row.legacy){
      const float r=corrected(legacyMv(old));
      assert(std::isfinite(r) && r<old);
      const float err=100.0f*std::fabs(r-row.dmm)/row.dmm;
      assert(err<1.0f);
      if(err>maxSample)maxSample=err;
      sum+=r;
    }
    const float avg=sum/3;
    const float e=100.0f*std::fabs(avg-row.dmm)/row.dmm;
    const float t=std::fabs(VishayNtcCurve::temperatureC(avg)-VishayNtcCurve::temperatureC(row.dmm));
    assert(e<0.7f && t<0.23f);
    if(e>maxMean)maxMean=e;
    if(t>maxTemp)maxTemp=t;
  }
  assert(std::isnan(corrected(NAN)));
  assert(std::isnan(corrected(0)));
  assert(std::isnan(corrected(6.8f)));
  assert(std::isnan(corrected(supply)));
  float prev=corrected(150.1f);
  for(float v=151;v<3000;v+=1){
    const float r=corrected(v);
    assert(std::isfinite(r) && r>prev);
    prev=r;
  }
  std::cout << "J7 replay 21 samples: max mean error " << maxMean
            << "%; max individual " << maxSample << "%; max nominal T error "
            << maxTemp << " C; invalid inputs and monotonicity PASS (host only)\n";
}
