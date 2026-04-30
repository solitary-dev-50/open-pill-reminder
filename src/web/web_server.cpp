// SPDX-License-Identifier: MIT

#include "web/web_server.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "app/config_manager.h"
#include "app/record_manager.h"
#include "app/reminder_manager.h"
#include "app_config.h"
#include "system/storage.h"
#include "system/time_service.h"

namespace {

const char* methodName(HTTPMethod method) {
  switch (method) {
    case HTTP_GET:
      return "GET";
    case HTTP_POST:
      return "POST";
    case HTTP_PUT:
      return "PUT";
    case HTTP_DELETE:
      return "DELETE";
    default:
      return "OTHER";
  }
}

}  // namespace

AppWebServer::AppWebServer() : _server(AppConfig::kHttpPort) {}

bool AppWebServer::begin(ConfigManager& configManager, RecordManager& recordManager,
                         ReminderManager& reminderManager, TimeService& timeService,
                         Storage& storage) {
  _configManager = &configManager;
  _recordManager = &recordManager;
  _reminderManager = &reminderManager;
  _timeService = &timeService;
  _storage = &storage;

  registerRoutes();
  _server.begin();
  Serial.printf("[网页] HTTP 服务已启动，端口 %u。\n", AppConfig::kHttpPort);
  return true;
}

void AppWebServer::update() {
  _server.handleClient();
}

void AppWebServer::registerRoutes() {
  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/app.css", HTTP_GET, [this]() {
    Serial.println("[网页] 请求静态文件 /app.css。");
    if (!sendFileFromLittleFs("/app.css", "text/css")) {
      _server.send(404, "text/plain", "app.css not found");
    }
  });
  _server.on("/app.js", HTTP_GET, [this]() {
    Serial.println("[网页] 请求静态文件 /app.js。");
    if (!sendFileFromLittleFs("/app.js", "application/javascript")) {
      _server.send(404, "text/plain", "app.js not found");
    }
  });

  _server.on("/api/status", HTTP_GET, [this]() { handleGetStatus(); });
  _server.on("/api/config", HTTP_GET, [this]() { handleGetConfig(); });
  _server.on("/api/config", HTTP_POST, [this]() { handlePostConfig(); });
  _server.on("/api/records/today", HTTP_GET, [this]() { handleGetTodayRecords(); });
  _server.on("/api/time/sync", HTTP_POST, [this]() { handleTimeSync(); });
  _server.on("/api/test/buzzer", HTTP_POST, [this]() { handleTestBuzzer(); });
  _server.on("/api/test/led", HTTP_POST, [this]() { handleTestLed(); });
  _server.on("/api/test/oled", HTTP_POST, [this]() { handleTestOled(); });
  _server.on("/api/test/trigger", HTTP_POST, [this]() { handleTriggerReminder(); });
  _server.onNotFound([this]() { handleNotFound(); });
}

void AppWebServer::handleRoot() {
  Serial.println("[网页] 请求首页 /。");
  if (sendFileFromLittleFs("/index.html", "text/html")) {
    Serial.println("[网页] 已从 LittleFS 返回 /index.html。");
    return;
  }

  Serial.println("[网页] 未找到 /index.html，返回兜底提示页。");
  const char* fallback =
      "<!doctype html><html><head><meta charset='utf-8'><title>小药记</title></head>"
      "<body><h1>小药记</h1><p>请先上传 LittleFS 页面文件。</p></body>"
      "</html>";
  _server.send(200, "text/html; charset=utf-8", fallback);
}

void AppWebServer::handleGetStatus() {
  Serial.printf("[网页] %s %s\n", methodName(_server.method()), _server.uri().c_str());

  JsonDocument doc;
  doc["app"] = "小药记";
  doc["state"] = (_reminderManager != nullptr) ? _reminderManager->getStateName() : "未知";
  doc["current_time"] = (_timeService != nullptr) ? _timeService->getTimeString() : "--";
  doc["current_date"] = (_timeService != nullptr) ? _timeService->getDateString() : "--";
  doc["time_ready"] = (_timeService != nullptr) ? _timeService->isTimeValid() : false;
  doc["time_status"] =
      (_timeService != nullptr) ? _timeService->getTimeStatusString() : "未同步";
  doc["time_source"] = (_timeService != nullptr) ? _timeService->sourceName() : "未知";
  doc["fs_ready"] = (_storage != nullptr) ? _storage->isReady() : false;
  doc["next_reminder"] =
      (_reminderManager != nullptr) ? _reminderManager->getNextReminderText() : "无";
  doc["active_reminder_id"] =
      (_reminderManager != nullptr) ? _reminderManager->getActiveReminderId() : "";
  doc["active_reminder_time"] =
      (_reminderManager != nullptr) ? _reminderManager->getActiveReminderTime() : "";
  doc["confirmed_count"] =
      (_reminderManager != nullptr) ? _reminderManager->getConfirmedCount() : 0;
  doc["total_count"] = (_reminderManager != nullptr) ? _reminderManager->getTotalCount() : 0;
  doc["ap_ssid"] = AppConfig::kAccessPointSsid;
  doc["ip"] = WiFi.softAPIP().toString();

  String payload;
  serializeJson(doc, payload);
  sendJson(200, payload);
}

void AppWebServer::handleGetConfig() {
  Serial.printf("[网页] %s %s\n", methodName(_server.method()), _server.uri().c_str());

  if (_configManager == nullptr) {
    sendJson(500, "{\"error\":\"配置管理器不可用\"}");
    return;
  }

  sendJson(200, _configManager->toJson(true));
}

void AppWebServer::handlePostConfig() {
  const String body = _server.arg("plain");
  Serial.printf("[网页] %s %s，body_length=%u。\n", methodName(_server.method()),
                _server.uri().c_str(), static_cast<unsigned>(body.length()));

  if (_configManager == nullptr) {
    sendJson(500, "{\"error\":\"配置管理器不可用\"}");
    return;
  }

  if (body.isEmpty()) {
    sendJson(400, "{\"error\":\"请求体为空\"}");
    return;
  }

  String errorMessage;
  if (!_configManager->applyJson(body, errorMessage)) {
    JsonDocument doc;
    doc["error"] = errorMessage;
    String payload;
    serializeJson(doc, payload);
    sendJson(400, payload);
    return;
  }

  JsonDocument doc;
  doc["ok"] = true;
  String payload;
  serializeJson(doc, payload);
  if (_reminderManager != nullptr) {
    _reminderManager->onConfigChanged();
  }
  sendJson(200, payload);
}

void AppWebServer::handleGetTodayRecords() {
  Serial.printf("[网页] %s %s\n", methodName(_server.method()), _server.uri().c_str());

  if (_recordManager == nullptr) {
    sendJson(500, "{\"error\":\"记录管理器不可用\"}");
    return;
  }

  sendJson(200, _recordManager->toJsonToday(true));
}

void AppWebServer::handleTimeSync() {
  const String body = _server.arg("plain");
  Serial.printf("[网页] %s %s，body_length=%u。\n", methodName(_server.method()),
                _server.uri().c_str(), static_cast<unsigned>(body.length()));

  if (_timeService == nullptr) {
    sendJson(500, "{\"error\":\"时间服务不可用\"}");
    return;
  }

  if (body.isEmpty()) {
    sendJson(400, "{\"error\":\"请求体为空\"}");
    return;
  }

  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.printf("[网页] 时间同步请求 JSON 无效：%s\n", error.c_str());
    sendJson(400, "{\"error\":\"JSON 无效\"}");
    return;
  }

  const int64_t epoch = doc["epoch"] | 0;
  const int32_t timezoneOffsetMinutes = doc["timezone_offset_minutes"] | 0;
  Serial.printf("[网页] 时间同步参数：epoch=%lld，timezone_offset_minutes=%ld。\n",
                static_cast<long long>(epoch), static_cast<long>(timezoneOffsetMinutes));

  if (!_timeService->syncFromBrowser(static_cast<time_t>(epoch), timezoneOffsetMinutes)) {
    sendJson(400, "{\"error\":\"时间戳无效\"}");
    return;
  }

  if (_reminderManager != nullptr) {
    _reminderManager->onConfigChanged();
  }

  sendJson(200, "{\"ok\":true}");
}

void AppWebServer::handleTestBuzzer() {
  Serial.printf("[网页] %s %s\n", methodName(_server.method()), _server.uri().c_str());
  if (_reminderManager != nullptr) {
    _reminderManager->triggerBuzzerTest();
  }
  sendJson(200, "{\"ok\":true}");
}

void AppWebServer::handleTestLed() {
  Serial.printf("[网页] %s %s\n", methodName(_server.method()), _server.uri().c_str());
  if (_reminderManager != nullptr) {
    _reminderManager->triggerLedTest();
  }
  sendJson(200, "{\"ok\":true}");
}

void AppWebServer::handleTestOled() {
  Serial.printf("[网页] %s %s\n", methodName(_server.method()), _server.uri().c_str());
  if (_reminderManager != nullptr) {
    _reminderManager->triggerOledTest();
  }
  sendJson(200, "{\"ok\":true}");
}

void AppWebServer::handleTriggerReminder() {
  Serial.printf("[网页] %s %s\n", methodName(_server.method()), _server.uri().c_str());
  if (_reminderManager != nullptr) {
    _reminderManager->triggerTestReminder();
  }
  sendJson(200, "{\"ok\":true}");
}

void AppWebServer::handleNotFound() {
  Serial.printf("[网页] 404 %s\n", _server.uri().c_str());
  _server.send(404, "text/plain; charset=utf-8", "未找到页面");
}

void AppWebServer::sendJson(int statusCode, const String& payload) {
  if (statusCode >= 400) {
    Serial.printf("[网页] 返回错误 %d，payload=%s\n", statusCode, payload.c_str());
  }
  _server.send(statusCode, "application/json; charset=utf-8", payload);
}

bool AppWebServer::sendFileFromLittleFs(const char* path, const char* contentType) {
  if (_storage == nullptr) {
    Serial.printf("[网页] LittleFS 文件发送失败，存储模块为空，path=%s。\n", path);
    return false;
  }

  if (!_storage->isReady()) {
    Serial.printf("[网页] LittleFS 文件发送失败，存储未就绪，path=%s。\n", path);
    return false;
  }

  if (!LittleFS.exists(path)) {
    Serial.printf("[网页] LittleFS 文件不存在：%s。\n", path);
    return false;
  }

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.printf("[网页] 打开静态文件失败：%s。\n", path);
    return false;
  }

  Serial.printf("[网页] 发送静态文件 %s，大小=%u，类型=%s。\n", path,
                static_cast<unsigned>(file.size()), contentType);
  _server.streamFile(file, contentType);
  file.close();
  return true;
}
