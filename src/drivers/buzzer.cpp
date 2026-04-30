// SPDX-License-Identifier: MIT

#include "drivers/buzzer.h"

#include "app_config.h"

void Buzzer::begin() {
  pinMode(AppConfig::kBuzzerPin, OUTPUT);
  setOutput(false);
  Serial.println("[蜂鸣器] 已就绪。");
}

void Buzzer::update() {
  const uint32_t now = millis();

  switch (_mode) {
    case Mode::Off:
      return;
    case Mode::Test:
      if (now - _modeStartedAt >= _testDurationMs) {
        stop();
      }
      return;
    case Mode::Alert:
      if (_outputOn && now - _lastToggleAt >= 250) {
        setOutput(false);
        _lastToggleAt = now;
      } else if (!_outputOn && now - _lastToggleAt >= 750) {
        setOutput(true);
        _lastToggleAt = now;
      }
      return;
  }
}

void Buzzer::test(uint32_t durationMs) {
  _mode = Mode::Test;
  _modeStartedAt = millis();
  _lastToggleAt = _modeStartedAt;
  _testDurationMs = durationMs;
  setOutput(true);
}

void Buzzer::startAlert() {
  _mode = Mode::Alert;
  _modeStartedAt = millis();
  _lastToggleAt = _modeStartedAt;
  setOutput(true);
}

void Buzzer::stop() {
  _mode = Mode::Off;
  setOutput(false);
}

bool Buzzer::isActive() const {
  return _mode != Mode::Off;
}

void Buzzer::setOutput(bool on) {
  _outputOn = on;
  digitalWrite(AppConfig::kBuzzerPin, on ? HIGH : LOW);
}
