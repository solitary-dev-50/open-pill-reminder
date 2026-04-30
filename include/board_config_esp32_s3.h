#pragma once

#include <Arduino.h>

namespace BoardConfig {

constexpr char kBoardFamily[] = "ESP32-S3";
constexpr char kBoardEnvName[] = "esp32-s3-devkitc-1";

// Default pin map for ESP32-S3-DevKitC-1 bring-up.
// Chosen to keep wiring simple and avoid USB, BOOT, UART download, and onboard RGB pins.
constexpr uint8_t kI2cSdaPin = 8;
constexpr uint8_t kI2cSclPin = 9;
constexpr uint8_t kBuzzerPin = 4;
constexpr uint8_t kButtonOkPin = 5;
constexpr uint8_t kButtonSetPin = 6;
constexpr uint8_t kLedPin = 7;
constexpr bool kHasOnboardWs2812 = true;
constexpr uint8_t kWs2812Pin = 38;
constexpr uint16_t kWs2812Count = 1;

}  // namespace BoardConfig
