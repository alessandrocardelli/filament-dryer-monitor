#pragma once
#include <cstdint>

// Small standalone read-only HTTP server on TCP 8081.
// The ESP-IDF server owns its own task. No network call, filesystem or HTTP
// request handling takes place inside the main heater/sensor loop.
// Does not modify or control heater GPIO, fan, OTA or settings.
class LiveTelemetryServer {
 public:
  struct Snapshot {
    uint32_t ms=0,cumulativeHeatMs=0;
    float airC=0,rh=0,ntcOhms=0,minSessionNtcOhms=0;
    uint16_t ramSamples=0;
    uint8_t fanPercent=0,phase=0,pulses=0,pulseFault=0,waitReason=0;
    bool airValid=false,ntcValid=false,heaterCommand=false,cycle=false,
         fault=false,sessionCsvSaved=false,sessionCsvFinal=false,
         logTruncated=false;
  };
  void begin();
  void publish(const Snapshot &snapshot);
 private:
  struct Internal;
  Internal *internal_=nullptr;
};
