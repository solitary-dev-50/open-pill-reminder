// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include <vector>

class Storage;

struct ReminderConfig {
  String id;
  String time;
  bool enabled = true;
};

struct DeviceConfig {
  uint16_t repeat_interval_minutes = 5;
  uint8_t max_repeat_count = 3;
  std::vector<ReminderConfig> reminders;
};

class ConfigManager {
 public:
  bool begin(Storage& storage);
  const DeviceConfig& getConfig() const;
  String toJson(bool pretty = false) const;
  bool applyJson(const String& json, String& errorMessage);
  bool saveCurrent();

  static bool isValidTimeString(const String& value);

 private:
  bool loadOrCreateDefault();
  void loadDefaults();
  bool parseJsonDocument(const String& json, DeviceConfig& outConfig, String& errorMessage) const;
  bool validateConfig(const DeviceConfig& config, String& errorMessage) const;
  String buildJson(const DeviceConfig& config, bool pretty) const;

  Storage* _storage = nullptr;
  DeviceConfig _config;
};
