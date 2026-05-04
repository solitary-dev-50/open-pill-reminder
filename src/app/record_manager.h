// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include <vector>

struct ReminderConfig;
struct DeviceConfig;

class Storage;
class TimeService;

struct DailyRecordItem {
  String reminder_id;
  String scheduled_time;
  String status;
  uint8_t repeat_count_used = 0;
  String confirmed_at;
};

struct DailyRecord {
  String date;
  std::vector<DailyRecordItem> items;
};

class RecordManager {
 public:
  bool begin(Storage& storage, TimeService& timeService);
  void update();
  bool syncWithConfig(const DeviceConfig& config);
  bool updateRecordItem(const String& reminderId, const String& scheduledTime, const String& status,
                        uint8_t repeatCountUsed, const String& confirmedAt = "");
  String getTodayPath() const;
  String toJsonToday(bool pretty = true) const;
  size_t getConfirmedCount() const;
  size_t getTotalCount() const;
  const DailyRecord& todayRecord() const;

 private:
  bool ensureTodayRecord();
  bool loadTodayRecord();
  bool saveTodayRecord() const;
  DailyRecordItem* findItemByReminderId(const String& reminderId);
  const DailyRecordItem* findItemByReminderId(const String& reminderId) const;

  Storage* _storage = nullptr;
  TimeService* _timeService = nullptr;
  String _activeDate;
  DailyRecord _todayRecord;
};
