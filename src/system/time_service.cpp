// SPDX-License-Identifier: MIT

#include "system/time_service.h"

#include <cstdio>

bool TimeService::begin() {
  _ready = false;
  _baseEpoch = 0;
  _baseMillis = millis();
  _timezoneOffsetMinutes = 0;
  _sourceName = "unsynced";

  Serial.println("[时间] 时间未设置，等待浏览器同步。");
  return true;
}

bool TimeService::syncFromBrowser(time_t epochUtc, int32_t timezoneOffsetMinutes) {
  Serial.printf("[时间] 收到浏览器同步请求，epoch=%lld，时区偏移=%ld 分钟。\n",
                static_cast<long long>(epochUtc), static_cast<long>(timezoneOffsetMinutes));

  if (epochUtc <= 0) {
    Serial.println("[时间] 浏览器同步失败：epoch 无效。");
    return false;
  }

  _baseEpoch = epochUtc;
  _baseMillis = millis();
  _timezoneOffsetMinutes = timezoneOffsetMinutes;
  _sourceName = "browser_sync";
  _ready = true;

  Serial.printf("[时间] 已同步，本地时间: %s，UTC: %lld，时区偏移: %ld 分钟。\n",
                getDateTimeString().c_str(), static_cast<long long>(getNowUtc()),
                static_cast<long>(_timezoneOffsetMinutes));
  return true;
}

bool TimeService::isTimeValid() const {
  return _ready;
}

bool TimeService::isReady() const {
  return isTimeValid();
}

time_t TimeService::getNowUtc() const {
  if (!isTimeValid()) {
    return 0;
  }

  const uint32_t elapsedSeconds = (millis() - _baseMillis) / 1000;
  return _baseEpoch + elapsedSeconds;
}

time_t TimeService::getNowLocal() const {
  if (!isTimeValid()) {
    return 0;
  }
  return getNowUtc() + (_timezoneOffsetMinutes * 60);
}

time_t TimeService::getNow() const {
  return getNowLocal();
}

int32_t TimeService::timezoneOffsetMinutes() const {
  return _timezoneOffsetMinutes;
}

String TimeService::sourceName() const {
  return _sourceName;
}

String TimeService::getTimeStatusString() const {
  return isTimeValid() ? "已同步" : "未同步";
}

String TimeService::getTimeString() const {
  return formatWithPattern("%H:%M:%S");
}

String TimeService::getDateString() const {
  return formatWithPattern("%Y-%m-%d");
}

String TimeService::getDateTimeString() const {
  return formatWithPattern("%Y-%m-%d %H:%M:%S");
}

String TimeService::formatWithPattern(const char* pattern) const {
  if (!isTimeValid()) {
    return "--";
  }

  time_t now = getNowLocal();
  struct tm currentTm = {};
  gmtime_r(&now, &currentTm);

  char buffer[32] = {};
  strftime(buffer, sizeof(buffer), pattern, &currentTm);
  return String(buffer);
}

time_t TimeService::nowEpoch() const {
  return getNowUtc();
}

time_t TimeService::nowLocalEpoch() const {
  return getNowLocal();
}

String TimeService::formatTime() const {
  return getTimeString();
}

String TimeService::formatDate() const {
  return getDateString();
}

String TimeService::formatDateTime() const {
  return getDateTimeString();
}
