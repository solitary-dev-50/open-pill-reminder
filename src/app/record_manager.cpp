// SPDX-License-Identifier: MIT

#include "app/record_manager.h"

#include <ArduinoJson.h>

#include "app/config_manager.h"
#include "app_config.h"
#include "system/storage.h"
#include "system/time_service.h"

namespace {

void logRecordItem(const DailyRecordItem& item, const char* prefix) {
  Serial.printf(
      "[记录] %s reminder_id=%s, scheduled_time=%s, status=%s, repeat_count_used=%u, "
      "confirmed_at=%s\n",
      prefix, item.reminder_id.c_str(), item.scheduled_time.c_str(), item.status.c_str(),
      static_cast<unsigned>(item.repeat_count_used),
      item.confirmed_at.isEmpty() ? "(null)" : item.confirmed_at.c_str());
}

}  // namespace

bool RecordManager::begin(Storage& storage, TimeService& timeService) {
  _storage = &storage;
  _timeService = &timeService;

  Serial.println("[记录] 记录管理器开始初始化。");
  if (_timeService == nullptr || !_timeService->isTimeValid()) {
    Serial.println("[记录] 当前时间未同步，今日记录将在同步后创建。");
    return false;
  }

  return ensureTodayRecord();
}

void RecordManager::update() {
  if (_timeService == nullptr || !_timeService->isTimeValid()) {
    return;
  }

  const String currentDate = _timeService->getDateString();
  if (currentDate != _activeDate) {
    Serial.printf("[记录] 检测到日期变化：%s -> %s，重新加载今日记录。\n", _activeDate.c_str(),
                  currentDate.c_str());
    loadTodayRecord();
  }
}

bool RecordManager::syncWithConfig(const DeviceConfig& config) {
  if (_timeService == nullptr || !_timeService->isTimeValid()) {
    Serial.println("[记录] 时间无效，跳过与配置同步。");
    return false;
  }

  if (_activeDate != _timeService->getDateString()) {
    loadTodayRecord();
  }

  Serial.printf("[记录] 正在同步今日记录与配置，提醒数=%u，日期=%s。\n",
                static_cast<unsigned>(config.reminders.size()), _activeDate.c_str());

  std::vector<DailyRecordItem> updatedItems;
  updatedItems.reserve(config.reminders.size());

  for (const ReminderConfig& reminder : config.reminders) {
    DailyRecordItem item;
    DailyRecordItem* existing = findItemByReminderId(reminder.id);
    if (existing != nullptr) {
      item = *existing;
    } else {
      item.reminder_id = reminder.id;
      item.repeat_count_used = 0;
      item.confirmed_at = "";
    }

    item.reminder_id = reminder.id;
    item.scheduled_time = reminder.time;

    const bool finalState = (item.status == "confirmed" || item.status == "expired");
    if (!finalState) {
      item.status = reminder.enabled ? "pending" : "disabled";
      item.repeat_count_used = 0;
      item.confirmed_at = "";
    }

    logRecordItem(item, "同步项");
    updatedItems.push_back(item);
  }

  _todayRecord.items = updatedItems;
  return saveTodayRecord();
}

bool RecordManager::updateRecordItem(const String& reminderId, const String& scheduledTime,
                                     const String& status, uint8_t repeatCountUsed,
                                     const String& confirmedAt) {
  if (_timeService == nullptr || !_timeService->isTimeValid()) {
    Serial.println("[记录] 时间无效，拒绝写入记录项。");
    return false;
  }

  if (_activeDate != _timeService->getDateString()) {
    loadTodayRecord();
  }

  DailyRecordItem* item = findItemByReminderId(reminderId);
  if (item == nullptr) {
    DailyRecordItem newItem;
    newItem.reminder_id = reminderId;
    newItem.scheduled_time = scheduledTime;
    newItem.status = status;
    newItem.repeat_count_used = repeatCountUsed;
    newItem.confirmed_at = confirmedAt;
    _todayRecord.items.push_back(newItem);
    logRecordItem(newItem, "新增记录");
  } else {
    item->scheduled_time = scheduledTime;
    item->status = status;
    item->repeat_count_used = repeatCountUsed;
    item->confirmed_at = confirmedAt;
    logRecordItem(*item, "更新记录");
  }

  return saveTodayRecord();
}

String RecordManager::getTodayPath() const {
  if (_timeService == nullptr || !_timeService->isTimeValid()) {
    return String(AppConfig::kRecordsDir) + "/unknown-date.json";
  }
  return String(AppConfig::kRecordsDir) + "/" + _timeService->getDateString() + ".json";
}

String RecordManager::toJsonToday(bool pretty) const {
  JsonDocument doc;
  doc["date"] = _todayRecord.date;

  JsonArray items = doc["items"].to<JsonArray>();
  for (const DailyRecordItem& item : _todayRecord.items) {
    JsonObject jsonItem = items.add<JsonObject>();
    jsonItem["reminder_id"] = item.reminder_id;
    jsonItem["scheduled_time"] = item.scheduled_time;
    jsonItem["status"] = item.status;
    jsonItem["repeat_count_used"] = item.repeat_count_used;
    if (item.confirmed_at.isEmpty()) {
      jsonItem["confirmed_at"] = nullptr;
    } else {
      jsonItem["confirmed_at"] = item.confirmed_at;
    }
  }

  String json;
  if (pretty) {
    serializeJsonPretty(doc, json);
  } else {
    serializeJson(doc, json);
  }
  return json;
}

size_t RecordManager::getConfirmedCount() const {
  size_t count = 0;
  for (const DailyRecordItem& item : _todayRecord.items) {
    if (item.status == "confirmed") {
      ++count;
    }
  }
  return count;
}

size_t RecordManager::getTotalCount() const {
  return _todayRecord.items.size();
}

const DailyRecord& RecordManager::todayRecord() const {
  return _todayRecord;
}

bool RecordManager::ensureTodayRecord() {
  if (_storage == nullptr || _timeService == nullptr || !_timeService->isTimeValid()) {
    Serial.println("[记录] 无法创建今日记录：存储或时间未就绪。");
    return false;
  }

  _activeDate = _timeService->getDateString();
  const String path = getTodayPath();
  Serial.printf("[记录] 检查今日记录文件：%s\n", path.c_str());
  if (_storage->exists(path.c_str())) {
    Serial.println("[记录] 今日记录已存在，直接加载。");
    return loadTodayRecord();
  }

  _todayRecord = {};
  _todayRecord.date = _activeDate;
  const bool saved = saveTodayRecord();
  Serial.printf("[记录] 创建 %s：%s\n", path.c_str(), saved ? "成功" : "失败");
  return saved;
}

bool RecordManager::loadTodayRecord() {
  if (_storage == nullptr || _timeService == nullptr || !_timeService->isTimeValid()) {
    Serial.println("[记录] 无法加载今日记录：存储或时间未就绪。");
    return false;
  }

  _activeDate = _timeService->getDateString();
  const String path = getTodayPath();
  if (!_storage->exists(path.c_str())) {
    Serial.printf("[记录] 今日记录不存在，准备新建：%s\n", path.c_str());
    _todayRecord = {};
    _todayRecord.date = _activeDate;
    return saveTodayRecord();
  }

  String json;
  if (!_storage->readText(path.c_str(), json)) {
    Serial.printf("[记录] 读取今日记录失败：%s\n", path.c_str());
    return false;
  }

  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.printf("[记录] 解析 %s 失败，正在重建。原因：%s\n", path.c_str(), error.c_str());
    _todayRecord = {};
    _todayRecord.date = _activeDate;
    return saveTodayRecord();
  }

  _todayRecord = {};
  _todayRecord.date = String(doc["date"] | _activeDate);
  JsonArray items = doc["items"].as<JsonArray>();
  for (JsonObject jsonItem : items) {
    DailyRecordItem item;
    item.reminder_id = String(jsonItem["reminder_id"] | "");
    item.scheduled_time = String(jsonItem["scheduled_time"] | "");
    item.status = String(jsonItem["status"] | "pending");
    item.repeat_count_used = jsonItem["repeat_count_used"] | 0;
    item.confirmed_at =
        jsonItem["confirmed_at"].isNull() ? "" : String(jsonItem["confirmed_at"] | "");
    _todayRecord.items.push_back(item);
  }

  Serial.printf("[记录] 已加载今日记录：%s，条目数=%u。\n", path.c_str(),
                static_cast<unsigned>(_todayRecord.items.size()));
  return true;
}

bool RecordManager::saveTodayRecord() const {
  if (_storage == nullptr || !_storage->isReady()) {
    Serial.println("[记录] 存储未就绪，无法保存今日记录。");
    return false;
  }

  const String path = getTodayPath();
  const String json = toJsonToday(true);
  const bool saved = _storage->writeText(path.c_str(), json);
  Serial.printf("[记录] 保存今日记录到 %s：%s，条目数=%u，JSON 长度=%u 字节。\n", path.c_str(),
                saved ? "成功" : "失败", static_cast<unsigned>(_todayRecord.items.size()),
                static_cast<unsigned>(json.length()));
  return saved;
}

DailyRecordItem* RecordManager::findItemByReminderId(const String& reminderId) {
  for (DailyRecordItem& item : _todayRecord.items) {
    if (item.reminder_id == reminderId) {
      return &item;
    }
  }
  return nullptr;
}
