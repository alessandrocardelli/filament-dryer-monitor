#pragma once
#include <cstddef>
#include <cstdint>

// One-minute time-series recording for BOTH selected experimental and product
// paths. Sensor/NTC sampling and heater control remain at their original pace.
// Always record a phase, heater, fan, valid-sensor or fault transition, plus
// operator-forced checkpoints. NO flash writes here; only the RAM ring is used.
// 12h normal duration: 721 baseline points; 1400-record RAM budget leaves
// room for ordinary transitions. Extraordinary event frequency can overrun
// this ring and MUST be flagged by ThermalLog::truncated().
// Historical completed attended runs with 5-second original CSV sampling
// remain in docs/evidence/thermal and are not modified by this change.
namespace LogSamplingPolicy {
constexpr uint32_t kNormalSampleMs=60000U;
constexpr uint32_t kTrialSampleMs=60000U; // formerly 5000U, capped a cycle at ~1h57m
constexpr uint32_t kMaxDurationMs=12U*60U*60U*1000U;
constexpr std::size_t kCapacity=1400U;
static_assert(kMaxDurationMs/kNormalSampleMs+2U<kCapacity,
              "Normal 12h baseline must fit the RAM ring");
static_assert(kMaxDurationMs/kTrialSampleMs+2U<kCapacity,
              "The actually selected heater experiment must also fit 12h");
constexpr uint32_t sampleIntervalMs(bool attended){
  return attended?kTrialSampleMs:kNormalSampleMs;
}
constexpr bool shouldCapture(uint32_t nowMs,uint32_t lastSampleMs,
                             bool hasPrevious,bool operatorCheckpoint,
                             bool stateChanged,uint32_t intervalMs){
  return !hasPrevious||operatorCheckpoint||stateChanged||
         static_cast<uint32_t>(nowMs-lastSampleMs)>=intervalMs;
}
constexpr bool willTruncate(std::size_t existingCount,std::size_t capacity){
  return existingCount>=capacity;
}
} // namespace LogSamplingPolicy
