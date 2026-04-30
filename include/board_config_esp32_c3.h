#pragma once

#include <Arduino.h>

namespace BoardConfig {

constexpr char kBoardFamily[] = "ESP32-C3";
constexpr char kBoardEnvName[] = "esp32-c3-devkitm-1";

constexpr uint8_t kI2cSdaPin = 4;
constexpr uint8_t kI2cSclPin = 5;
constexpr uint8_t kBuzzerPin = 6;
constexpr uint8_t kButtonOkPin = 7;
constexpr uint8_t kButtonSetPin = 8;
constexpr uint8_t kLedPin = 10;
constexpr bool kHasOnboardWs2812 = false;
constexpr uint8_t kWs2812Pin = 255;
constexpr uint16_t kWs2812Count = 0;

}  // namespace BoardConfig
