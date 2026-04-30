// SPDX-License-Identifier: MIT

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "app/config_manager.h"
#include "app/record_manager.h"
#include "app/reminder_manager.h"
#include "app_config.h"
#include "drivers/buttons.h"
#include "drivers/buzzer.h"
#include "drivers/display_oled.h"
#include "drivers/led.h"
#include "system/storage.h"
#include "system/time_service.h"
#include "web/web_server.h"

namespace {

Storage g_storage;
TimeService g_timeService;
ConfigManager g_configManager;
RecordManager g_recordManager;
DisplayOled g_display;
Buzzer g_buzzer;
Led g_led;
Buttons g_buttons;
ReminderManager g_reminderManager;
AppWebServer g_webServer;

const char* buttonIdToString(ButtonId id) {
  switch (id) {
    case ButtonId::Ok:
      return "确认键";
    case ButtonId::SetBack:
      return "设置键";
  }
  return "未知按键";
}

const char* buttonEventTypeToString(ButtonEventType type) {
  switch (type) {
    case ButtonEventType::Pressed:
      return "按下";
    case ButtonEventType::Released:
      return "松开";
    case ButtonEventType::ShortPress:
      return "短按";
  }
  return "未知事件";
}

void printRuntimeDiagnostics() {
  Serial.printf("[系统] 芯片型号: %s\n", ESP.getChipModel());
  Serial.printf("[系统] 芯片版本: %u\n", static_cast<unsigned>(ESP.getChipRevision()));
  Serial.printf("[系统] CPU 频率: %u MHz\n", static_cast<unsigned>(ESP.getCpuFreqMHz()));
  Serial.printf("[系统] Flash 大小: %u 字节\n", static_cast<unsigned>(ESP.getFlashChipSize()));
  Serial.printf("[系统] 程序大小: %u 字节\n", static_cast<unsigned>(ESP.getSketchSize()));
  Serial.printf("[系统] 剩余堆内存: %u 字节\n", static_cast<unsigned>(ESP.getFreeHeap()));
}

void listLittleFsEntries() {
  if (!g_storage.isReady()) {
    Serial.println("[系统] LittleFS 未就绪，跳过文件列表。");
    return;
  }

  Serial.println("[系统] LittleFS 文件列表开始。");
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("[系统] 无法打开 LittleFS 根目录。");
    return;
  }

  File entry = root.openNextFile();
  if (!entry) {
    Serial.println("[系统] LittleFS 当前为空。");
  }
  while (entry) {
    Serial.printf("[系统] 文件: %s, 大小: %u\n", entry.path(),
                  static_cast<unsigned>(entry.size()));
    entry = root.openNextFile();
  }
  Serial.println("[系统] LittleFS 文件列表结束。");
}

void setupAccessPoint() {
  WiFi.mode(WIFI_AP);
  const bool started = WiFi.softAP(AppConfig::kAccessPointSsid, AppConfig::kAccessPointPassword);
  Serial.printf("[网络] 热点%s\n", started ? "启动成功" : "启动失败");
  Serial.printf("[网络] SSID: %s\n", AppConfig::kAccessPointSsid);
  Serial.printf("[网络] IP: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("[网络] MAC: %s\n", WiFi.softAPmacAddress().c_str());
}

void handleButtonEvents() {
  ButtonEvent event;
  while (g_buttons.popEvent(event)) {
    Serial.printf("[按键] %s %s\n", buttonIdToString(event.id), buttonEventTypeToString(event.type));

    if (event.type == ButtonEventType::ShortPress && event.id == ButtonId::Ok) {
      g_reminderManager.onOkShortPress();
      if (g_reminderManager.isAlertActive()) {
        Serial.println("[应用] 确认键短按已交由提醒管理器处理。");
      }
    }
  }
}

}  // namespace

void setup() {
  Serial.begin(AppConfig::kSerialBaudRate);
  delay(300);

  Serial.println();
  Serial.println("================================");
  Serial.println("小药记 Open Pill Reminder V0.1");
  Serial.printf("目标芯片: %s\n", AppConfig::kTargetName);
  Serial.printf("构建环境: %s\n", AppConfig::kTargetEnvName);
  Serial.println("第一阶段固件启动");
  Serial.println("================================");

  printRuntimeDiagnostics();

  g_led.begin();
  g_buzzer.begin();
  g_buttons.begin();
  g_display.begin();
  g_display.showBoot("启动中", "初始化存储...");

  g_storage.begin();
  listLittleFsEntries();

  g_timeService.begin();
  g_configManager.begin(g_storage);
  g_recordManager.begin(g_storage, g_timeService);
  g_reminderManager.begin(g_configManager, g_recordManager, g_timeService, g_display, g_buzzer,
                          g_led);

  setupAccessPoint();
  g_webServer.begin(g_configManager, g_recordManager, g_reminderManager, g_timeService,
                    g_storage);

  g_display.showBoot("系统就绪", WiFi.softAPIP().toString());
  Serial.println("[应用] 初始化完成。");
}

void loop() {
  g_buttons.update();
  handleButtonEvents();

  g_recordManager.update();
  g_reminderManager.update();
  g_webServer.update();

  delay(5);
}
