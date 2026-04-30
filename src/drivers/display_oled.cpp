// SPDX-License-Identifier: MIT

#include "drivers/display_oled.h"

#include <Wire.h>

#include "app_config.h"

DisplayOled::DisplayOled()
    : _display(AppConfig::kOledWidth, AppConfig::kOledHeight, &Wire, -1) {}

bool DisplayOled::begin() {
  Wire.begin(AppConfig::kI2cSdaPin, AppConfig::kI2cSclPin);

  _ready = _display.begin(SSD1306_SWITCHCAPVCC, AppConfig::kOledAddress);
  if (!_ready) {
    Serial.println("[显示] OLED 初始化失败。");
    return false;
  }

  _display.clearDisplay();
  _display.display();
  showBoot("OLED 就绪");
  Serial.println("[显示] OLED 已就绪。");
  return true;
}

bool DisplayOled::isReady() const {
  return _ready;
}

void DisplayOled::showBoot(const String& line1, const String& line2) {
  showStatus("小药记", line1, line2);
}

void DisplayOled::showStatus(const String& title, const String& line1, const String& line2) {
  if (!_ready) {
    return;
  }

  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);

  _display.setTextSize(2);
  _display.setCursor(0, 0);
  _display.println(title);

  _display.setTextSize(1);
  _display.setCursor(0, 24);
  _display.println(line1);
  _display.setCursor(0, 40);
  _display.println(line2);
  _display.display();
}
