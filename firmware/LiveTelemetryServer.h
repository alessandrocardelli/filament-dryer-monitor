#pragma once
#include <cstdint>
#include "RemoteControlPolicy.h"

// Standalone HTTP server on TCP 8081, background task.
// The ESP-IDF server owns its own task. No network call, filesystem or HTTP
// request handling takes place inside the main heater/sensor loop.
// Commands are authenticated and QUEUED. Only the application loop may
// validate/apply settings or STOP; there is NO remote START endpoint.
class LiveTelemetryServer {
 public:
  struct Snapshot {
    uint32_t ms=0,cumulativeHeatMs=0;
    float airC=0,rh=0,ntcOhms=0,minSessionNtcOhms=0;
    uint16_t ramSamples=0;
    uint8_t appState=0, profile=0;
    uint16_t targetC=0, durationMinutes=0;
    bool canEdit=false;
    uint8_t fanPercent=0,phase=0,pulses=0,pulseFault=0,waitReason=0;
    bool airValid=false,ntcValid=false,heaterCommand=false,cycle=false,
         fault=false,sessionCsvSaved=false,sessionCsvFinal=false,
         logTruncated=false;
  };
  void begin(const char *adminPassword);
  void publish(const Snapshot &snapshot);
  bool takeStopRequest();
  bool takeSettingsRequest(RemoteControlPolicy::Settings &out);
 private:
  struct Internal;
  Internal *internal_=nullptr;
};
