// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>
#include <WebServer.h>

class ConfigManager;
class RecordManager;
class ReminderManager;
class TimeService;
class Storage;

class AppWebServer {
 public:
  AppWebServer();
  bool begin(ConfigManager& configManager, RecordManager& recordManager,
             ReminderManager& reminderManager, TimeService& timeService, Storage& storage);
  void update();

 private:
  void registerRoutes();
  void handleRoot();
  void handleGetStatus();
  void handleGetConfig();
  void handlePostConfig();
  void handleGetTodayRecords();
  void handleTimeSync();
  void handleTestBuzzer();
  void handleTestLed();
  void handleTestOled();
  void handleTriggerReminder();
  void handleNotFound();
  void sendJson(int statusCode, const String& payload);
  bool sendFileFromLittleFs(const char* path, const char* contentType);
  ConfigManager& configManager() const;
  RecordManager& recordManager() const;
  ReminderManager& reminderManager() const;
  TimeService& timeService() const;
  Storage& storage() const;

  ConfigManager* _configManager = nullptr;
  RecordManager* _recordManager = nullptr;
  ReminderManager* _reminderManager = nullptr;
  TimeService* _timeService = nullptr;
  Storage* _storage = nullptr;

  WebServer _server;
};
