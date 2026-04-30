// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

class Buzzer {
 public:
  void begin();
  void update();
  void test(uint32_t durationMs = 300);
  void startAlert();
  void stop();
  bool isActive() const;

 private:
  enum class Mode {
    Off,
    Test,
    Alert,
  };

  void setOutput(bool on);

  Mode _mode = Mode::Off;
  bool _outputOn = false;
  uint32_t _modeStartedAt = 0;
  uint32_t _lastToggleAt = 0;
  uint32_t _testDurationMs = 0;
};
