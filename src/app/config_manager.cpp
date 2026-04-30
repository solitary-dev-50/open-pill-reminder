// SPDX-License-Identifier: MIT

#include "app/config_manager.h"

#include <ArduinoJson.h>

#include "app_config.h"
#include "system/storage.h"

namespace {

void logConfigSummary(const DeviceConfig& config, const char* prefix) {
  Serial.printf(
      "[配置] %s 提醒数=%u, repeat_interval_minutes=%u, max_repeat_count=%u\n", prefix,
      static_cast<unsigned>(config.reminders.size()),
      static_cast<unsigned>(config.repeat_interval_minutes),
      static_cast<unsigned>(config.max_repeat_count));

  for (size_t index = 0; index < config.reminders.size(); ++index) {
    const ReminderConfig& reminder = config.reminders[index];
    Serial.printf("[配置] 提醒[%u] id=%s, time=%s, enabled=%s\n", static_cast<unsigned>(index),
                  reminder.id.c_str(), reminder.time.c_str(), reminder.enabled ? "true" : "false");
  }
}

}  // namespace

bool ConfigManager::begin(Storage& storage) {
  _storage = &storage;
  Serial.println("[配置] 配置管理器开始初始化。");
  return loadOrCreateDefault();
}

const DeviceConfig& ConfigManager::getConfig() const {
  return _config;
}

String ConfigManager::toJson(bool pretty) const {
  return buildJson(_config, pretty);
}

bool ConfigManager::applyJson(const String& json, String& errorMessage) {
  Serial.printf("[配置] 收到配置更新请求，JSON 长度=%u 字节。\n",
                static_cast<unsigned>(json.length()));

  DeviceConfig newConfig;
  if (!parseJsonDocument(json, newConfig, errorMessage)) {
    Serial.printf("[配置] 配置更新失败：%s\n", errorMessage.c_str());
    return false;
  }

  _config = newConfig;
  logConfigSummary(_config, "应用新配置：");
  return saveCurrent();
}

bool ConfigManager::saveCurrent() {
  if (_storage == nullptr) {
    Serial.println("[配置] 未绑定存储模块。");
    return false;
  }

  const String json = buildJson(_config, true);
  const bool saved = _storage->writeText(AppConfig::kConfigPath, json);
  Serial.printf("[配置] 保存到 %s：%s，JSON 长度=%u 字节。\n", AppConfig::kConfigPath,
                saved ? "成功" : "失败", static_cast<unsigned>(json.length()));
  return saved;
}

bool ConfigManager::isValidTimeString(const String& value) {
  if (value.length() != 5 || value.charAt(2) != ':') {
    return false;
  }

  if (!isDigit(value.charAt(0)) || !isDigit(value.charAt(1)) || !isDigit(value.charAt(3)) ||
      !isDigit(value.charAt(4))) {
    return false;
  }

  const int hour = value.substring(0, 2).toInt();
  const int minute = value.substring(3, 5).toInt();
  return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

bool ConfigManager::loadOrCreateDefault() {
  if (_storage == nullptr) {
    Serial.println("[配置] 存储模块不可用，无法加载配置。");
    return false;
  }

  if (!_storage->exists(AppConfig::kConfigPath)) {
    Serial.printf("[配置] 未找到 %s，正在创建默认配置。\n", AppConfig::kConfigPath);
    loadDefaults();
    logConfigSummary(_config, "默认配置：");
    return saveCurrent();
  }

  String json;
  if (!_storage->readText(AppConfig::kConfigPath, json)) {
    Serial.printf("[配置] 读取 %s 失败，改用默认配置。\n", AppConfig::kConfigPath);
    loadDefaults();
    logConfigSummary(_config, "默认配置：");
    return saveCurrent();
  }

  String errorMessage;
  DeviceConfig loadedConfig;
  if (!parseJsonDocument(json, loadedConfig, errorMessage)) {
    Serial.printf("[配置] 配置无效，重置为默认值。原因：%s\n", errorMessage.c_str());
    loadDefaults();
    logConfigSummary(_config, "默认配置：");
    return saveCurrent();
  }

  _config = loadedConfig;
  Serial.printf("[配置] 配置加载成功，来源=%s。\n", AppConfig::kConfigPath);
  logConfigSummary(_config, "当前配置：");
  return true;
}

void ConfigManager::loadDefaults() {
  _config = {};
  _config.repeat_interval_minutes = AppConfig::kDefaultRepeatIntervalMinutes;
  _config.max_repeat_count = AppConfig::kDefaultMaxRepeatCount;
  _config.reminders.clear();
}

bool ConfigManager::parseJsonDocument(const String& json, DeviceConfig& outConfig,
                                      String& errorMessage) const {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    errorMessage = String("JSON 解析失败: ") + error.c_str();
    return false;
  }

  DeviceConfig parsedConfig;
  parsedConfig.repeat_interval_minutes =
      doc["repeat_interval_minutes"] | AppConfig::kDefaultRepeatIntervalMinutes;
  parsedConfig.max_repeat_count = doc["max_repeat_count"] | AppConfig::kDefaultMaxRepeatCount;

  JsonArray reminders = doc["reminders"].as<JsonArray>();
  if (doc["reminders"].isNull()) {
    errorMessage = "缺少 reminders 数组";
    return false;
  }

  for (JsonObject item : reminders) {
    ReminderConfig reminder;
    reminder.id = String(item["id"] | "");
    reminder.time = String(item["time"] | "");
    reminder.enabled = item["enabled"] | true;
    parsedConfig.reminders.push_back(reminder);
  }

  if (!validateConfig(parsedConfig, errorMessage)) {
    return false;
  }

  outConfig = parsedConfig;
  return true;
}

bool ConfigManager::validateConfig(const DeviceConfig& config, String& errorMessage) const {
  if (config.repeat_interval_minutes < 1 || config.repeat_interval_minutes > 1440) {
    errorMessage = "repeat_interval_minutes 必须在 1 到 1440 之间";
    return false;
  }

  if (config.max_repeat_count > 20) {
    errorMessage = "max_repeat_count 必须在 0 到 20 之间";
    return false;
  }

  if (config.reminders.size() > AppConfig::kMaxReminders) {
    errorMessage = "提醒数量过多";
    return false;
  }

  for (size_t index = 0; index < config.reminders.size(); ++index) {
    const ReminderConfig& reminder = config.reminders[index];
    if (reminder.id.isEmpty()) {
      errorMessage = "提醒 id 不能为空";
      return false;
    }

    if (!isValidTimeString(reminder.time)) {
      errorMessage = "提醒时间格式无效";
      return false;
    }

    for (size_t compareIndex = index + 1; compareIndex < config.reminders.size(); ++compareIndex) {
      if (config.reminders[compareIndex].id == reminder.id) {
        errorMessage = "提醒 id 重复";
        return false;
      }
    }
  }

  return true;
}

String ConfigManager::buildJson(const DeviceConfig& config, bool pretty) const {
  JsonDocument doc;
  doc["repeat_interval_minutes"] = config.repeat_interval_minutes;
  doc["max_repeat_count"] = config.max_repeat_count;

  JsonArray reminders = doc["reminders"].to<JsonArray>();
  for (const ReminderConfig& reminder : config.reminders) {
    JsonObject item = reminders.add<JsonObject>();
    item["id"] = reminder.id;
    item["time"] = reminder.time;
    item["enabled"] = reminder.enabled;
  }

  String json;
  if (pretty) {
    serializeJsonPretty(doc, json);
  } else {
    serializeJson(doc, json);
  }
  return json;
}
