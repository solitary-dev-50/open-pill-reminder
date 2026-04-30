// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

enum class ButtonId : uint8_t {
  Ok,
  SetBack,
};

enum class ButtonEventType : uint8_t {
  Pressed,
  Released,
  ShortPress,
};

struct ButtonEvent {
  ButtonId id;
  ButtonEventType type;
};

class Buttons {
 public:
  void begin();
  void update();
  bool popEvent(ButtonEvent& event);

 private:
  struct ButtonState {
    ButtonId id;
    uint8_t pin;
    bool stablePressed = false;
    bool lastRawPressed = false;
    uint32_t lastRawChangeAt = 0;
    uint32_t pressedAt = 0;
  };

  void pushEvent(ButtonId id, ButtonEventType type);
  void updateButton(ButtonState& state, uint32_t now);

  static constexpr uint8_t kEventQueueSize = 8;

  ButtonState _buttons[2] = {};
  ButtonEvent _eventQueue[kEventQueueSize] = {};
  uint8_t _queueHead = 0;
  uint8_t _queueTail = 0;
};
