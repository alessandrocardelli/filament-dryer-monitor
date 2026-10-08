#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "AppConfig.h"
#include "AppTypes.h"

class Sht45Sensor {
 public:
  explicit Sht45Sensor(TwoWire &wire) : wire_(wire) {}

  void begin(uint32_t nowMs) {
    nextMeasurementMs_ = nowMs;
    snapshot_ = SensorSnapshot{};
  }

  void update(uint32_t nowMs) {
    if (state_ == State::Idle) {
      if ((int32_t)(nowMs - nextMeasurementMs_) < 0) return;
      startMeasurement(nowMs);
      return;
    }

    if (state_ == State::Waiting && (int32_t)(nowMs - readyMs_) >= 0) {
      readMeasurement(nowMs);
    }
  }

  const SensorSnapshot &snapshot() const { return snapshot_; }

  bool healthy(uint32_t nowMs) const {
    return snapshot_.valid && (nowMs - snapshot_.lastSuccessMs) <= AppConfig::kSht45StaleMs;
  }

 private:
  enum class State : uint8_t { Idle, Waiting };
  TwoWire &wire_;
  State state_ = State::Idle;
  SensorSnapshot snapshot_{};
  uint32_t readyMs_ = 0;
  uint32_t nextMeasurementMs_ = 0;

  static uint8_t crc8(const uint8_t *data, size_t length) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < length; ++i) {
      crc ^= data[i];
      for (uint8_t bit = 0; bit < 8; ++bit) {
        crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x31)
                           : static_cast<uint8_t>(crc << 1);
      }
    }
    return crc;
  }

  void startMeasurement(uint32_t nowMs) {
    wire_.beginTransmission(AppConfig::kSht45Address);
    wire_.write(0xFD); // High-repeatability T/RH measurement, no sensor heater.
    if (wire_.endTransmission() == 0) {
      state_ = State::Waiting;
      readyMs_ = nowMs + AppConfig::kSht45ConversionMs;
    } else {
      snapshot_.valid = false;
      nextMeasurementMs_ = nowMs + AppConfig::kSht45PeriodMs;
    }
  }

  void readMeasurement(uint32_t nowMs) {
    uint8_t data[6] = {0};
    const uint8_t received = wire_.requestFrom(AppConfig::kSht45Address, static_cast<uint8_t>(6));
    for (uint8_t i = 0; i < received && i < 6; ++i) data[i] = wire_.read();

    const bool crcOk = received == 6 && crc8(&data[0], 2) == data[2] && crc8(&data[3], 2) == data[5];
    if (crcOk) {
      const uint16_t rawT = (static_cast<uint16_t>(data[0]) << 8) | data[1];
      const uint16_t rawRh = (static_cast<uint16_t>(data[3]) << 8) | data[4];
      snapshot_.temperatureC = -45.0f + 175.0f * (static_cast<float>(rawT) / 65535.0f);
      float rh = -6.0f + 125.0f * (static_cast<float>(rawRh) / 65535.0f);
      snapshot_.humidityRh = constrain(rh, 0.0f, 100.0f);
      snapshot_.valid = true;
      snapshot_.lastSuccessMs = nowMs;
    } else {
      snapshot_.valid = false;
    }

    state_ = State::Idle;
    nextMeasurementMs_ = nowMs + AppConfig::kSht45PeriodMs;
  }
};
