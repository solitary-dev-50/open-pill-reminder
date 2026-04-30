// SPDX-License-Identifier: MIT

#pragma once

#include <Adafruit_SSD1306.h>
#include <Arduino.h>

class DisplayOled {
 public:
  DisplayOled();

  bool begin();
  bool isReady() const;
  void showBoot(const String& line1, const String& line2 = "");
  void showStatus(const String& title, const String& line1, const String& line2);

 private:
  Adafruit_SSD1306 _display;
  bool _ready = false;
};
