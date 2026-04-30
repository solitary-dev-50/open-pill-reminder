// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#if defined(OPEN_PILL_TARGET_ESP32_C3)
#include "board_config_esp32_c3.h"
#elif defined(OPEN_PILL_TARGET_ESP32_S3)
#include "board_config_esp32_s3.h"
#else
#error "Define OPEN_PILL_TARGET_ESP32_C3 or OPEN_PILL_TARGET_ESP32_S3 in build_flags."
#endif

namespace AppConfig {

constexpr uint32_t kSerialBaudRate = 115200;

constexpr const char* kTargetName = BoardConfig::kBoardFamily;
constexpr const char* kTargetEnvName = BoardConfig::kBoardEnvName;

constexpr uint8_t kI2cSdaPin = BoardConfig::kI2cSdaPin;
constexpr uint8_t kI2cSclPin = BoardConfig::kI2cSclPin;
constexpr uint8_t kBuzzerPin = BoardConfig::kBuzzerPin;
constexpr uint8_t kButtonOkPin = BoardConfig::kButtonOkPin;
constexpr uint8_t kButtonSetPin = BoardConfig::kButtonSetPin;
constexpr uint8_t kLedPin = BoardConfig::kLedPin;
constexpr bool kUseWs2812 = BoardConfig::kHasOnboardWs2812;
constexpr uint8_t kWs2812Pin = BoardConfig::kWs2812Pin;
constexpr uint16_t kWs2812Count = BoardConfig::kWs2812Count;

constexpr uint8_t kOledAddress = 0x3C;
constexpr uint16_t kOledWidth = 128;
constexpr uint16_t kOledHeight = 64;

constexpr uint8_t kMaxReminders = 8;
constexpr uint16_t kDefaultRepeatIntervalMinutes = 5;
constexpr uint8_t kDefaultMaxRepeatCount = 3;

constexpr uint32_t kButtonDebounceMs = 30;
constexpr uint32_t kButtonLongPressMs = 800;

constexpr uint32_t kBuzzerTestDurationMs = 300;
constexpr uint32_t kLedTestDurationMs = 500;
constexpr uint32_t kTestAlertDurationMs = 15000;

constexpr char kConfigPath[] = "/config.json";
constexpr char kRecordsDir[] = "/records";

constexpr char kAccessPointSsid[] = "小药记";
constexpr char kAccessPointPassword[] = "12345678";
constexpr uint16_t kHttpPort = 80;

}  // namespace AppConfig
