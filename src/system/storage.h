// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

class Storage {
 public:
  bool begin();
  bool isReady() const;
  bool exists(const char* path) const;
  bool ensureDir(const char* path);
  bool readText(const char* path, String& outText) const;
  bool writeText(const char* path, const String& text);

 private:
  bool _ready = false;
};
