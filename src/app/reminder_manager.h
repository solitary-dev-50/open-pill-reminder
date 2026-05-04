// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include <chrono>
#include <vector>

class ConfigManager;
class RecordManager;
class TimeService;
class DisplayOled;
class Buzzer;
class Led;

class ReminderManager {
 public:
  bool begin(ConfigManager& configManager, RecordManager& recordManager, TimeService& timeService,
             DisplayOled& display, Buzzer& buzzer, Led& led);
  void update();
  void onConfigChanged();
  void onOkShortPress();

  void triggerBuzzerTest();
  void triggerLedTest();
  void triggerOledTest();
  void triggerTestReminder();
  void stopActiveAlert();

  bool isAlertActive() const;
  String getStateName() const;
  String getNextReminderText() const;
  String getActiveReminderId() const;
  String getActiveReminderTime() const;
  size_t getConfirmedCount() const;
  size_t getTotalCount() const;

 private:
  enum class RuntimeState : uint8_t {
    Disabled,
    WaitingToday,
    Alerting,
    WaitingRepeat,
    Confirmed,
    Expired,
  };

  enum class State : uint8_t {
    TimeNotSet,
    Idle,
    ReminderAlert,
    TestAlert,
  };

  struct ReminderRuntime {
    using EpochSeconds = std::chrono::seconds;

    String id;
    String time;
    bool enabled = true;
    RuntimeState state = RuntimeState::WaitingToday;
    EpochSeconds scheduledEpoch{0};
    EpochSeconds nextRepeatEpoch{0};
    uint8_t repeatCountUsed = 0;
  };

  void rebuildForToday(bool forceRecordSync);
  void updateCurrentDay();
  void updateRuntimeStateMachine();
  void startReminderAlert(size_t runtimeIndex, bool isRepeat);
  void finishReminderAsExpired(size_t runtimeIndex);
  void confirmActiveReminder();
  void renderDisplay(bool force = false);
  String formatNowTimeShort() const;
  int currentMinutesOfDay() const;
  int minutesFromTimeString(const String& value) const;
  ReminderRuntime::EpochSeconds buildTodayEpochForTime(const String& hhmm) const;
  bool hasAnyActiveReminderDue() const;
  bool isFinalRuntimeState(RuntimeState state) const;
  void updateAlertOutputs();
  void startAlertOutputs();
  void stopAlertOutputs();
  void showIdleVisualState();
  void showTimeNotSetVisualState();

  ConfigManager* _configManager = nullptr;
  RecordManager* _recordManager = nullptr;
  TimeService* _timeService = nullptr;
  DisplayOled* _display = nullptr;
  Buzzer* _buzzer = nullptr;
  Led* _led = nullptr;

  State _state = State::Idle;
  uint32_t _alertUntilMs = 0;
  uint32_t _lastRenderMs = 0;
  bool _forceRender = true;
  String _activeDate;
  int _activeReminderIndex = -1;
  std::vector<ReminderRuntime> _runtimes;

  String _temporaryTitle;
  String _temporaryLine1;
  String _temporaryLine2;
  uint32_t _temporaryUntilMs = 0;
};
