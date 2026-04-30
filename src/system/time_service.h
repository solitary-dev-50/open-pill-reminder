#pragma once

#include <Arduino.h>
#include <time.h>

class TimeService {
 public:
  bool begin();
  bool syncFromBrowser(time_t epochUtc, int32_t timezoneOffsetMinutes);
  bool isTimeValid() const;
  time_t getNow() const;
  time_t getNowUtc() const;
  time_t getNowLocal() const;
  int32_t timezoneOffsetMinutes() const;
  String sourceName() const;
  String getTimeStatusString() const;
  String getTimeString() const;
  String getDateString() const;
  String getDateTimeString() const;

  // Backward-compatible wrappers for current modules.
  bool isReady() const;
  time_t nowEpoch() const;
  time_t nowLocalEpoch() const;
  String formatTime() const;
  String formatDate() const;
  String formatDateTime() const;

 private:
  String formatWithPattern(const char* pattern) const;

  bool _ready = false;
  time_t _baseEpoch = 0;
  uint32_t _baseMillis = 0;
  int32_t _timezoneOffsetMinutes = 0;
  String _sourceName = "unsynced";
};
