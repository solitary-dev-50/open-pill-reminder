#include "drivers/buttons.h"

#include "app_config.h"

void Buttons::begin() {
  _buttons[0].id = ButtonId::Ok;
  _buttons[0].pin = AppConfig::kButtonOkPin;
  _buttons[1].id = ButtonId::SetBack;
  _buttons[1].pin = AppConfig::kButtonSetPin;

  for (ButtonState& button : _buttons) {
    pinMode(button.pin, INPUT_PULLUP);
    const bool rawPressed = (digitalRead(button.pin) == LOW);
    button.stablePressed = rawPressed;
    button.lastRawPressed = rawPressed;
    button.lastRawChangeAt = millis();
  }

  Serial.println("[按键] 已就绪。");
}

void Buttons::update() {
  const uint32_t now = millis();
  for (ButtonState& button : _buttons) {
    updateButton(button, now);
  }
}

bool Buttons::popEvent(ButtonEvent& event) {
  if (_queueHead == _queueTail) {
    return false;
  }

  event = _eventQueue[_queueHead];
  _queueHead = (_queueHead + 1) % kEventQueueSize;
  return true;
}

void Buttons::pushEvent(ButtonId id, ButtonEventType type) {
  const uint8_t nextTail = (_queueTail + 1) % kEventQueueSize;
  if (nextTail == _queueHead) {
    return;
  }

  _eventQueue[_queueTail] = {id, type};
  _queueTail = nextTail;
}

void Buttons::updateButton(ButtonState& state, uint32_t now) {
  const bool rawPressed = (digitalRead(state.pin) == LOW);
  if (rawPressed != state.lastRawPressed) {
    state.lastRawPressed = rawPressed;
    state.lastRawChangeAt = now;
  }

  if (now - state.lastRawChangeAt < AppConfig::kButtonDebounceMs) {
    return;
  }

  if (state.stablePressed == state.lastRawPressed) {
    return;
  }

  state.stablePressed = state.lastRawPressed;
  if (state.stablePressed) {
    state.pressedAt = now;
    pushEvent(state.id, ButtonEventType::Pressed);
    return;
  }

  pushEvent(state.id, ButtonEventType::Released);
  pushEvent(state.id, ButtonEventType::ShortPress);
}
