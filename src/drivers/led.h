// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

class Led {
 public:
  enum class StatusStyle : uint8_t {
    TimeNotSet,
    Idle,
  };

  void begin();
  void update();

  void showTimeNotSetStatus();
  void showIdleStatus();
  void pulseConfirmed(uint32_t durationMs = 800);
  void test(uint32_t durationMs = 500);
  void startAlert();
  void stopAlert();

  bool isActive() const;

 private:
  enum class Mode : uint8_t {
    Status,
    Test,
    Alert,
    ConfirmPulse,
  };

  void applyStatusOutputs(uint32_t now);
  void applyOutputs(bool gpioOn, uint8_t red, uint8_t green, uint8_t blue);
  void writeWs2812Color(uint8_t red, uint8_t green, uint8_t blue);

  Mode _mode = Mode::Status;
  StatusStyle _statusStyle = StatusStyle::TimeNotSet;
  bool _gpioOutputOn = false;
  uint32_t _modeStartedAt = 0;
  uint32_t _lastToggleAt = 0;
  uint32_t _effectDurationMs = 0;
  bool _pixelReady = false;
};
