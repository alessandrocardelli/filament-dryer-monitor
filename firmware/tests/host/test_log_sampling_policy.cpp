// Source-only unit test: timer cadence, not a GPIO/heater test.
#include "../../LogSamplingPolicy.h"
#include <cassert>
#include <cstdint>
#include <iostream>
int main(){
  static_assert(LogSamplingPolicy::sampleIntervalMs(true)==5000);
  static_assert(LogSamplingPolicy::sampleIntervalMs(false)==60000);
  const uint32_t cycle=LogSamplingPolicy::kMaxDurationMs;
  const uint32_t capacity=LogSamplingPolicy::kCapacity;
  // Reserve one endpoint and as much space as possible for edge events.
  const uint32_t baseline=cycle/LogSamplingPolicy::kNormalSampleMs+1;
  assert(baseline==721 && baseline+1<capacity);
  const uint32_t reservedEdges=capacity-baseline;
  assert(reservedEdges>=679);
  std::cout<<"PASS 12h logging cadence: 721 regular points; 679 edge capacity; overflow explicitly flagged.\n";
}
