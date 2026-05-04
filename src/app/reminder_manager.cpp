// SPDX-License-Identifier: MIT

#include "reminder_manager.h"

#include <algorithm>
#include <chrono>
#include <limits>

#include "config_manager.h"
#include "record_manager.h"
#include "app_config.h"
#include "../drivers/buzzer.h"
#include "../drivers/display_oled.h"
#include "../drivers/led.h"
#include "../system/time_service.h"

bool ReminderManager::begin(ConfigManager& configManager, RecordManager& recordManager,
                            TimeService& timeService, DisplayOled& display, Buzzer& buzzer,
                            Led& led) {
  _configManager = &configManager;
  _recordManager = &recordManager;
  _timeService = &timeService;
  _display = &display;
  _buzzer = &buzzer;
  _led = &led;
  _state = _timeService->isTimeValid() ? State::Idle : State::TimeNotSet;
  _forceRender = true;

  if (_state == State::TimeNotSet) {
    showTimeNotSetVisualState();
  } else {
    showIdleVisualState();
  }

  Serial.printf("[提醒] 提醒管理器初始化，初始状态=%s。\n", getStateName().c_str());
  rebuildForToday(true);
  renderDisplay(true);
  return true;
}

void ReminderManager::update() {
  updateAlertOutputs();

  updateCurrentDay();
  updateRuntimeStateMachine();

  const uint32_t now = millis();
  if (_state == State::TestAlert && now >= _alertUntilMs) {
    Serial.println("[提醒] 测试提醒到时，自动停止。");
    stopActiveAlert();
  }

  if (_temporaryUntilMs > 0 && now >= _temporaryUntilMs) {
    Serial.println("[提醒] 临时显示内容已超时，恢复主界面。");
    _temporaryUntilMs = 0;
    _temporaryTitle = "";
    _temporaryLine1 = "";
    _temporaryLine2 = "";
    _forceRender = true;
  }

  renderDisplay();
}

void ReminderManager::onConfigChanged() {
  if (_configManager != nullptr) {
    const DeviceConfig& config = _configManager->getConfig();
    Serial.printf(
        "[提醒] 检测到配置变化，提醒数=%u，repeat_interval_minutes=%u，max_repeat_count=%u。\n",
        static_cast<unsigned>(config.reminders.size()),
        static_cast<unsigned>(config.repeat_interval_minutes),
        static_cast<unsigned>(config.max_repeat_count));
  } else {
    Serial.println("[提醒] 检测到配置变化，但配置管理器不可用。");
  }

  rebuildForToday(true);
}

void ReminderManager::onOkShortPress() {
  Serial.printf("[提醒] 收到确认键短按，当前状态=%s。\n", getStateName().c_str());

  if (_state == State::TestAlert) {
    stopActiveAlert();
    return;
  }

  if (_state == State::ReminderAlert) {
    confirmActiveReminder();
    return;
  }

  Serial.println("[提醒] 当前没有可确认的提醒。");
}

void ReminderManager::triggerBuzzerTest() {
  Serial.println("[提醒] 触发蜂鸣器测试。");
  if (_buzzer != nullptr) {
    _buzzer->test();
  }
}

void ReminderManager::triggerLedTest() {
  Serial.println("[提醒] 触发指示灯测试。");
  if (_led != nullptr) {
    _led->test();
  }
}

void ReminderManager::triggerOledTest() {
  Serial.println("[提醒] 触发 OLED 测试显示。");
  _temporaryTitle = "OLED 测试";
  _temporaryLine1 = "显示正常";
  _temporaryLine2 = "小药记";
  _temporaryUntilMs = millis() + 2000;
  _forceRender = true;
}

void ReminderManager::triggerTestReminder() {
  Serial.println("[提醒] 触发测试提醒。");
  startAlertOutputs();

  _state = State::TestAlert;
  _alertUntilMs = millis() + AppConfig::kTestAlertDurationMs;
  _forceRender = true;
}

void ReminderManager::stopActiveAlert() {
  Serial.printf("[提醒] 停止当前提醒，当前状态=%s，active_index=%d。\n", getStateName().c_str(),
                _activeReminderIndex);

  stopAlertOutputs();

  if (_state == State::TestAlert) {
    _state =
        _timeService != nullptr && _timeService->isTimeValid() ? State::Idle : State::TimeNotSet;
  } else if (_state == State::ReminderAlert) {
    _state = State::Idle;
  }

  if (_state == State::TimeNotSet) {
    showTimeNotSetVisualState();
  } else {
    showIdleVisualState();
  }

  _activeReminderIndex = -1;
  _forceRender = true;
}

bool ReminderManager::isAlertActive() const {
  return _state == State::TestAlert || _state == State::ReminderAlert;
}

String ReminderManager::getStateName() const {
  switch (_state) {
    case State::TimeNotSet:
      return "时间未设置";
    case State::Idle:
      return "待机";
    case State::ReminderAlert:
      return "提醒中";
    case State::TestAlert:
      return "测试提醒";
  }
  return "未知状态";
}

String ReminderManager::getNextReminderText() const {
  if (_configManager == nullptr || _timeService == nullptr || !_timeService->isTimeValid()) {
    return "--:--";
  }

  const DeviceConfig& config = _configManager->getConfig();
  if (config.reminders.empty()) {
    return "无";
  }

  const int nowMinutes = currentMinutesOfDay();
  int bestDelta = std::numeric_limits<int>::max();
  String bestTime = "无";

  for (const ReminderConfig& reminder : config.reminders) {
    if (!reminder.enabled) {
      continue;
    }

    const int reminderMinutes = minutesFromTimeString(reminder.time);
    if (reminderMinutes < 0) {
      continue;
    }

    int delta = reminderMinutes - nowMinutes;
    if (delta < 0) {
      delta += 24 * 60;
    }

    if (delta < bestDelta) {
      bestDelta = delta;
      bestTime = reminder.time;
    }
  }

  return bestTime;
}

String ReminderManager::getActiveReminderId() const {
  if (_activeReminderIndex < 0 || _activeReminderIndex >= static_cast<int>(_runtimes.size())) {
    return "";
  }
  return _runtimes[_activeReminderIndex].id;
}

String ReminderManager::getActiveReminderTime() const {
  if (_activeReminderIndex < 0 || _activeReminderIndex >= static_cast<int>(_runtimes.size())) {
    return "";
  }
  return _runtimes[_activeReminderIndex].time;
}

size_t ReminderManager::getConfirmedCount() const {
  return (_recordManager != nullptr) ? _recordManager->getConfirmedCount() : 0;
}

size_t ReminderManager::getTotalCount() const {
  return (_recordManager != nullptr) ? _recordManager->getTotalCount() : 0;
}

void ReminderManager::rebuildForToday(bool forceRecordSync) {
  auto runtimeStateName = [](RuntimeState state) -> const char* {
    switch (state) {
      case RuntimeState::Disabled:
        return "disabled";
      case RuntimeState::WaitingToday:
        return "waiting_today";
      case RuntimeState::Alerting:
        return "alerting";
      case RuntimeState::WaitingRepeat:
        return "waiting_repeat";
      case RuntimeState::Confirmed:
        return "confirmed";
      case RuntimeState::Expired:
        return "expired";
    }
    return "unknown";
  };

  Serial.printf("[提醒] 开始重建今日提醒，forceRecordSync=%s。\n",
                forceRecordSync ? "true" : "false");

  stopAlertOutputs();

  _runtimes.clear();
  _activeReminderIndex = -1;

  if (_configManager == nullptr || _timeService == nullptr || !_timeService->isTimeValid()) {
    _state = State::TimeNotSet;
    showTimeNotSetVisualState();
    _forceRender = true;
    Serial.println("[提醒] 时间尚未同步，提醒状态设置为 TIME_NOT_SET。");
    return;
  }

  _activeDate = _timeService->getDateString();
  const DeviceConfig& config = _configManager->getConfig();
  Serial.printf("[提醒] 当前日期=%s，配置提醒数=%u。\n", _activeDate.c_str(),
                static_cast<unsigned>(config.reminders.size()));

  if (_recordManager != nullptr && forceRecordSync) {
    const bool synced = _recordManager->syncWithConfig(config);
    Serial.printf("[提醒] 与今日记录同步：%s。\n", synced ? "成功" : "失败");
  }

  _runtimes.reserve(config.reminders.size());
  for (const ReminderConfig& reminder : config.reminders) {
    ReminderRuntime runtime;
    runtime.id = reminder.id;
    runtime.time = reminder.time;
    runtime.enabled = reminder.enabled;
    runtime.scheduledEpoch = buildTodayEpochForTime(reminder.time);
    runtime.state = reminder.enabled ? RuntimeState::WaitingToday : RuntimeState::Disabled;
    runtime.nextRepeatEpoch = ReminderRuntime::EpochSeconds{0};
    runtime.repeatCountUsed = 0;

    if (_recordManager != nullptr) {
      const DailyRecord& record = _recordManager->todayRecord();
      for (const DailyRecordItem& item : record.items) {
        if (item.reminder_id != reminder.id) {
          continue;
        }

        runtime.repeatCountUsed = item.repeat_count_used;
        if (!reminder.enabled || item.status == "disabled") {
          runtime.state = RuntimeState::Disabled;
        } else if (item.status == "confirmed") {
          runtime.state = RuntimeState::Confirmed;
        } else if (item.status == "expired") {
          runtime.state = RuntimeState::Expired;
        } else {
          runtime.state = RuntimeState::WaitingToday;
        }
        break;
      }
    }

    Serial.printf("[提醒] 构建提醒 id=%s, time=%s, enabled=%s, scheduled_epoch=%lld, state=%s, "
                  "repeat_count_used=%u\n",
                  runtime.id.c_str(), runtime.time.c_str(), runtime.enabled ? "true" : "false",
                  static_cast<long long>(runtime.scheduledEpoch.count()),
                  runtimeStateName(runtime.state),
                  static_cast<unsigned>(runtime.repeatCountUsed));
    _runtimes.push_back(runtime);
  }

  std::sort(_runtimes.begin(), _runtimes.end(),
            [](const ReminderRuntime& left, const ReminderRuntime& right) {
              if (left.scheduledEpoch == right.scheduledEpoch) {
                return left.id < right.id;
              }
              return left.scheduledEpoch < right.scheduledEpoch;
            });

  _state = State::Idle;
  showIdleVisualState();
  _forceRender = true;
  Serial.printf("[提醒] 今日提醒重建完成，运行时条目数=%u，状态=%s。\n",
                static_cast<unsigned>(_runtimes.size()), getStateName().c_str());
}

void ReminderManager::updateCurrentDay() {
  if (_timeService == nullptr) {
    return;
  }

  if (!_timeService->isTimeValid()) {
    if (_state != State::TimeNotSet) {
      Serial.println("[提醒] 时间状态从有效变为无效，切换到 TIME_NOT_SET。");
      _state = State::TimeNotSet;
      showTimeNotSetVisualState();
      _forceRender = true;
    }
    return;
  }

  const String currentDate = _timeService->getDateString();
  if (_activeDate != currentDate) {
    Serial.printf("[提醒] 检测到跨天：%s -> %s。\n", _activeDate.c_str(), currentDate.c_str());
    if (_recordManager != nullptr) {
      _recordManager->update();
    }
    rebuildForToday(true);
  }
}

void ReminderManager::updateRuntimeStateMachine() {
  if (_timeService == nullptr || !_timeService->isTimeValid()) {
    return;
  }

  if (_state == State::TestAlert) {
    return;
  }

  const auto now = _timeService->nowUtcSeconds();

  if (_state == State::ReminderAlert && _activeReminderIndex >= 0 &&
      _activeReminderIndex < static_cast<int>(_runtimes.size())) {
    ReminderRuntime& runtime = _runtimes[_activeReminderIndex];
    if (millis() >= _alertUntilMs) {
      Serial.printf("[提醒] 提醒超时 id=%s, repeat_count_used=%u, max_repeat_count=%u。\n",
                    runtime.id.c_str(), static_cast<unsigned>(runtime.repeatCountUsed),
                    static_cast<unsigned>(_configManager->getConfig().max_repeat_count));

      if (runtime.repeatCountUsed < _configManager->getConfig().max_repeat_count) {
        runtime.repeatCountUsed++;
        runtime.state = RuntimeState::WaitingRepeat;
        runtime.nextRepeatEpoch =
            now + std::chrono::minutes{_configManager->getConfig().repeat_interval_minutes};
        Serial.printf("[提醒] 进入重复等待 id=%s, next_repeat_epoch=%lld。\n", runtime.id.c_str(),
                      static_cast<long long>(runtime.nextRepeatEpoch.count()));
        if (_recordManager != nullptr) {
          _recordManager->updateRecordItem(runtime.id, runtime.time, "pending",
                                           runtime.repeatCountUsed);
        }
      } else {
        Serial.printf("[提醒] 已达到最大重复次数，标记为 expired，id=%s。\n", runtime.id.c_str());
        finishReminderAsExpired(static_cast<size_t>(_activeReminderIndex));
      }

      if (_state == State::ReminderAlert) {
        stopAlertOutputs();
        _state = State::Idle;
        _activeReminderIndex = -1;
        showIdleVisualState();
        _forceRender = true;
      }
    }
    return;
  }

  for (size_t index = 0; index < _runtimes.size(); ++index) {
    ReminderRuntime& runtime = _runtimes[index];
    if (!runtime.enabled || runtime.state == RuntimeState::Disabled ||
        isFinalRuntimeState(runtime.state)) {
      continue;
    }

    if (runtime.state == RuntimeState::WaitingToday && now >= runtime.scheduledEpoch) {
      Serial.printf("[提醒] 到达首次提醒时间，id=%s, time=%s。\n", runtime.id.c_str(),
                    runtime.time.c_str());
      startReminderAlert(index, false);
      return;
    }

    if (runtime.state == RuntimeState::WaitingRepeat && now >= runtime.nextRepeatEpoch) {
      Serial.printf("[提醒] 到达重复提醒时间，id=%s, time=%s, repeat_count_used=%u。\n",
                    runtime.id.c_str(), runtime.time.c_str(),
                    static_cast<unsigned>(runtime.repeatCountUsed));
      startReminderAlert(index, true);
      return;
    }
  }

  if (_state == State::TimeNotSet) {
    _state = State::Idle;
    showIdleVisualState();
    _forceRender = true;
    Serial.println("[提醒] 时间已恢复有效，切换回待机状态。");
  }
}

void ReminderManager::startReminderAlert(size_t runtimeIndex, bool isRepeat) {
  if (runtimeIndex >= _runtimes.size()) {
    Serial.printf("[提醒] startReminderAlert 越界，runtimeIndex=%u。\n",
                  static_cast<unsigned>(runtimeIndex));
    return;
  }

  ReminderRuntime& runtime = _runtimes[runtimeIndex];
  runtime.state = RuntimeState::Alerting;
  _activeReminderIndex = static_cast<int>(runtimeIndex);
  _state = State::ReminderAlert;
  _alertUntilMs = millis() + AppConfig::kTestAlertDurationMs;

  Serial.printf("[提醒] 开始%s提醒，id=%s, time=%s, repeat_count_used=%u, alert_until_ms=%u。\n",
                isRepeat ? "重复" : "首次", runtime.id.c_str(), runtime.time.c_str(),
                static_cast<unsigned>(runtime.repeatCountUsed),
                static_cast<unsigned>(_alertUntilMs));

  startAlertOutputs();

  if (_recordManager != nullptr) {
    _recordManager->updateRecordItem(runtime.id, runtime.time, "pending", runtime.repeatCountUsed);
  }

  _forceRender = true;
}

void ReminderManager::finishReminderAsExpired(size_t runtimeIndex) {
  if (runtimeIndex >= _runtimes.size()) {
    Serial.printf("[提醒] finishReminderAsExpired 越界，runtimeIndex=%u。\n",
                  static_cast<unsigned>(runtimeIndex));
    return;
  }

  ReminderRuntime& runtime = _runtimes[runtimeIndex];
  runtime.state = RuntimeState::Expired;
  Serial.printf("[提醒] 提醒过期，id=%s, time=%s, repeat_count_used=%u。\n", runtime.id.c_str(),
                runtime.time.c_str(), static_cast<unsigned>(runtime.repeatCountUsed));
  if (_recordManager != nullptr) {
    _recordManager->updateRecordItem(runtime.id, runtime.time, "expired", runtime.repeatCountUsed);
  }
}

void ReminderManager::confirmActiveReminder() {
  if (_activeReminderIndex < 0 || _activeReminderIndex >= static_cast<int>(_runtimes.size())) {
    Serial.println("[提醒] confirmActiveReminder 时没有有效的 activeReminderIndex。");
    return;
  }

  ReminderRuntime& runtime = _runtimes[_activeReminderIndex];
  runtime.state = RuntimeState::Confirmed;

  const String confirmedAt = formatNowTimeShort();
  Serial.printf("[提醒] 用户确认提醒，id=%s, scheduled_time=%s, confirmed_at=%s, "
                "repeat_count_used=%u。\n",
                runtime.id.c_str(), runtime.time.c_str(), confirmedAt.c_str(),
                static_cast<unsigned>(runtime.repeatCountUsed));

  if (_recordManager != nullptr) {
    _recordManager->updateRecordItem(runtime.id, runtime.time, "confirmed", runtime.repeatCountUsed,
                                     confirmedAt);
  }

  if (_buzzer != nullptr) {
    _buzzer->stop();
  }
  if (_led != nullptr) {
    _led->pulseConfirmed();
  }

  _temporaryTitle = "已确认";
  _temporaryLine1 = runtime.time;
  _temporaryLine2 = confirmedAt;
  _temporaryUntilMs = millis() + 2000;
  _state = State::Idle;
  _activeReminderIndex = -1;
  _forceRender = true;
}

void ReminderManager::renderDisplay(bool force) {
  if (_display == nullptr || !_display->isReady()) {
    return;
  }

  const uint32_t now = millis();
  if (!force && !_forceRender && now - _lastRenderMs < 1000) {
    return;
  }

  _lastRenderMs = now;
  _forceRender = false;

  if (_temporaryUntilMs > 0) {
    _display->showStatus(_temporaryTitle, _temporaryLine1, _temporaryLine2);
    return;
  }

  if (_state == State::TimeNotSet) {
    _display->showStatus("小药记", "时间未设置", "打开网页控制台");
    return;
  }

  if (_state == State::TestAlert) {
    _display->showStatus("提醒测试", "测试提醒中", "按确认键停止");
    return;
  }

  if (_state == State::ReminderAlert && _activeReminderIndex >= 0 &&
      _activeReminderIndex < static_cast<int>(_runtimes.size())) {
    const ReminderRuntime& runtime = _runtimes[_activeReminderIndex];
    _display->showStatus("请服药", runtime.time, "按确认键记录");
    return;
  }

  const String timeText = (_timeService != nullptr) ? _timeService->getTimeString() : "--";
  const String nextText = "下次: " + getNextReminderText();
  const String summary = String("已确认: ") + getConfirmedCount() + "/" + getTotalCount();
  _display->showStatus("小药记", timeText + " " + summary, nextText);
}

String ReminderManager::formatNowTimeShort() const {
  if (_timeService == nullptr || !_timeService->isTimeValid()) {
    return "--:--";
  }

  const String formatted = _timeService->getTimeString();
  return formatted.substring(0, 5);
}

int ReminderManager::currentMinutesOfDay() const {
  if (_timeService == nullptr || !_timeService->isTimeValid()) {
    return 0;
  }

  return _timeService->localMinutesOfDay();
}

int ReminderManager::minutesFromTimeString(const String& value) const {
  uint8_t hour = 0;
  uint8_t minute = 0;
  if (!ConfigManager::parseTimeString(value, hour, minute)) {
    return -1;
  }

  return hour * 60 + minute;
}

ReminderManager::ReminderRuntime::EpochSeconds ReminderManager::buildTodayEpochForTime(
    const String& hhmm) const {
  if (_timeService == nullptr || !_timeService->isTimeValid() ||
      !ConfigManager::isValidTimeString(hhmm)) {
    return ReminderRuntime::EpochSeconds{0};
  }

  uint8_t hour = 0;
  uint8_t minute = 0;
  if (!ConfigManager::parseTimeString(hhmm, hour, minute)) {
    return ReminderRuntime::EpochSeconds{0};
  }

  return ReminderRuntime::EpochSeconds{_timeService->localTimeToUtcEpoch(hour, minute)};
}

bool ReminderManager::hasAnyActiveReminderDue() const {
  for (const ReminderRuntime& runtime : _runtimes) {
    if (runtime.state == RuntimeState::Alerting || runtime.state == RuntimeState::WaitingRepeat) {
      return true;
    }
  }
  return false;
}

bool ReminderManager::isFinalRuntimeState(RuntimeState state) const {
  return state == RuntimeState::Confirmed || state == RuntimeState::Expired;
}

void ReminderManager::updateAlertOutputs() {
  if (_buzzer != nullptr) {
    _buzzer->update();
  }
  if (_led != nullptr) {
    _led->update();
  }
}

void ReminderManager::startAlertOutputs() {
  if (_buzzer != nullptr) {
    _buzzer->startAlert();
  }
  if (_led != nullptr) {
    _led->startAlert();
  }
}

void ReminderManager::stopAlertOutputs() {
  if (_buzzer != nullptr) {
    _buzzer->stop();
  }
  if (_led != nullptr) {
    _led->stopAlert();
  }
}

void ReminderManager::showIdleVisualState() {
  if (_led != nullptr) {
    _led->showIdleStatus();
  }
}

void ReminderManager::showTimeNotSetVisualState() {
  if (_led != nullptr) {
    _led->showTimeNotSetStatus();
  }
}
