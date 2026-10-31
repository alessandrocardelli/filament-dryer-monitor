#pragma once
#include <cstdint>
// Logging never changes heater control or sensor sampling.
// 12h normal baseline: 721 samples at 60s; remaining room for state edges.
// If unusually many edges exceed 1400 samples, explicitly mark truncation.
// The historical attended trial retains its 5s sampling interval.
namespace LogSamplingPolicy {
constexpr uint32_t kNormalSampleMs=60000U;
constexpr uint32_t kTrialSampleMs=5000U;
constexpr uint32_t kMaxDurationMs=12U*60U*60U*1000U;
constexpr uint32_t kCapacity=1400U;
static_assert(kMaxDurationMs/kNormalSampleMs+2<kCapacity,
              "Normal 12h baseline must fit the RAM ring");
constexpr uint32_t sampleIntervalMs(bool attended){
  return attended?kTrialSampleMs:kNormalSampleMs;
}
} // namespace LogSamplingPolicy
