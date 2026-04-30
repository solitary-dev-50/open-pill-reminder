// SPDX-License-Identifier: MIT

#include "drivers/led.h"

#include "app_config.h"
#include "esp32-hal-rgb-led.h"

void Led::begin() {
  pinMode(AppConfig::kLedPin, OUTPUT);
  digitalWrite(AppConfig::kLedPin, LOW);

  if (AppConfig::kUseWs2812 && AppConfig::kWs2812Count > 0) {
    _pixelReady = true;
    writeWs2812Color(0, 0, 0);
    Serial.printf("[指示灯] 板载 WS2812 已启用，GPIO=%u，数量=%u，驱动=neopixelWrite。\n",
                  static_cast<unsigned>(AppConfig::kWs2812Pin),
                  static_cast<unsigned>(AppConfig::kWs2812Count));
  } else {
    _pixelReady = false;
    Serial.println("[指示灯] 板载 WS2812 未启用。");
  }

  _mode = Mode::Status;
  _statusStyle = StatusStyle::TimeNotSet;
  _modeStartedAt = millis();
  _lastToggleAt = _modeStartedAt;
  applyStatusOutputs(_modeStartedAt);

  Serial.printf("[指示灯] 外接指示灯已就绪，GPIO=%u。\n", static_cast<unsigned>(AppConfig::kLedPin));
  Serial.println("[指示灯] 状态灯规则：未同步=黄闪，待机=绿常亮，测试=蓝色，提醒=红闪，确认=绿短闪。");
}

void Led::update() {
  const uint32_t now = millis();

  switch (_mode) {
    case Mode::Status:
      applyStatusOutputs(now);
      return;
    case Mode::Test:
      if (now - _modeStartedAt >= _effectDurationMs) {
        Serial.println("[指示灯] 测试结束，恢复基础状态显示。");
        _mode = Mode::Status;
        applyStatusOutputs(now);
      }
      return;
    case Mode::Alert:
      if (now - _lastToggleAt >= 250) {
        const bool turnOn = !_gpioOutputOn;
        applyOutputs(turnOn, turnOn ? 24 : 0, 0, 0);
        _lastToggleAt = now;
      }
      return;
    case Mode::ConfirmPulse: {
      if (now - _modeStartedAt >= _effectDurationMs) {
        Serial.println("[指示灯] 确认短闪结束，恢复基础状态显示。");
        _mode = Mode::Status;
        applyStatusOutputs(now);
        return;
      }

      const uint32_t phase = ((now - _modeStartedAt) / 150) % 2;
      const bool turnOn = (phase == 0);
      applyOutputs(turnOn, 0, turnOn ? 32 : 0, 0);
      return;
    }
  }
}

void Led::showTimeNotSetStatus() {
  if (_statusStyle != StatusStyle::TimeNotSet || _mode != Mode::Status) {
    Serial.println("[指示灯] 切换到“时间未设置”状态灯。");
  }
  _statusStyle = StatusStyle::TimeNotSet;
  if (_mode == Mode::Status) {
    _modeStartedAt = millis();
    _lastToggleAt = _modeStartedAt;
    applyStatusOutputs(_modeStartedAt);
  }
}

void Led::showIdleStatus() {
  if (_statusStyle != StatusStyle::Idle || _mode != Mode::Status) {
    Serial.println("[指示灯] 切换到“待机”状态灯。");
  }
  _statusStyle = StatusStyle::Idle;
  if (_mode == Mode::Status) {
    applyStatusOutputs(millis());
  }
}

void Led::pulseConfirmed(uint32_t durationMs) {
  Serial.printf("[指示灯] 执行确认短闪，持续 %u ms。\n", static_cast<unsigned>(durationMs));
  _mode = Mode::ConfirmPulse;
  _modeStartedAt = millis();
  _effectDurationMs = durationMs;
  applyOutputs(true, 0, 32, 0);
}

void Led::test(uint32_t durationMs) {
  Serial.printf("[指示灯] 执行测试，持续 %u ms。\n", static_cast<unsigned>(durationMs));
  _mode = Mode::Test;
  _modeStartedAt = millis();
  _effectDurationMs = durationMs;
  applyOutputs(true, 0, 0, 32);
}

void Led::startAlert() {
  Serial.println("[指示灯] 开始提醒闪烁。");
  _mode = Mode::Alert;
  _modeStartedAt = millis();
  _lastToggleAt = _modeStartedAt;
  applyOutputs(true, 24, 0, 0);
}

void Led::stopAlert() {
  Serial.println("[指示灯] 停止提醒闪烁，恢复基础状态。");
  _mode = Mode::Status;
  applyStatusOutputs(millis());
}

bool Led::isActive() const {
  return _mode != Mode::Status || _statusStyle == StatusStyle::TimeNotSet;
}

void Led::applyStatusOutputs(uint32_t now) {
  switch (_statusStyle) {
    case StatusStyle::TimeNotSet:
      if (now - _lastToggleAt >= 800) {
        _lastToggleAt = now;
        applyOutputs(!_gpioOutputOn, !_gpioOutputOn ? 24 : 0, !_gpioOutputOn ? 16 : 0, 0);
      } else if (_modeStartedAt == _lastToggleAt) {
        applyOutputs(true, 24, 16, 0);
      }
      return;
    case StatusStyle::Idle:
      applyOutputs(false, 0, 16, 0);
      return;
  }
}

void Led::applyOutputs(bool gpioOn, uint8_t red, uint8_t green, uint8_t blue) {
  _gpioOutputOn = gpioOn;
  digitalWrite(AppConfig::kLedPin, gpioOn ? HIGH : LOW);
  writeWs2812Color(red, green, blue);
}

void Led::writeWs2812Color(uint8_t red, uint8_t green, uint8_t blue) {
  if (!_pixelReady) {
    return;
  }

  neopixelWrite(AppConfig::kWs2812Pin, red, green, blue);
}
