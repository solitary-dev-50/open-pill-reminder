#include "system/storage.h"

#include <LittleFS.h>

#include "app_config.h"

namespace {

String getParentDir(const char* path) {
  if (path == nullptr) {
    return "/";
  }

  const String fullPath(path);
  const int lastSlash = fullPath.lastIndexOf('/');
  if (lastSlash <= 0) {
    return "/";
  }

  return fullPath.substring(0, lastSlash);
}

}  // namespace

bool Storage::begin() {
  Serial.println("[存储] 正在挂载 LittleFS...");
  _ready = LittleFS.begin(false);
  if (!_ready) {
    Serial.println("[存储] LittleFS 挂载失败。");
    Serial.println("[存储] 已禁用自动格式化，避免因挂载失败清空配置。");
    return false;
  }

  Serial.println("[存储] LittleFS 挂载成功。");
  if (!ensureDir(AppConfig::kRecordsDir)) {
    Serial.printf("[存储] 记录目录初始化失败: %s\n", AppConfig::kRecordsDir);
  }
  return true;
}

bool Storage::isReady() const {
  return _ready;
}

bool Storage::exists(const char* path) const {
  if (!_ready) {
    return false;
  }
  return LittleFS.exists(path);
}

bool Storage::ensureDir(const char* path) {
  if (!_ready || path == nullptr) {
    return false;
  }

  if (String(path) == "/") {
    return true;
  }

  if (LittleFS.exists(path)) {
    File existing = LittleFS.open(path, "r");
    const bool isDir = existing && existing.isDirectory();
    existing.close();
    return isDir;
  }

  const bool created = LittleFS.mkdir(path);
  Serial.printf("[存储] 创建目录 %s: %s\n", path, created ? "成功" : "失败");
  return created;
}

bool Storage::readText(const char* path, String& outText) const {
  outText = "";
  if (!_ready) {
    return false;
  }

  File file = LittleFS.open(path, "r");
  if (!file) {
    return false;
  }

  outText = file.readString();
  file.close();
  return true;
}

bool Storage::writeText(const char* path, const String& text) {
  if (!_ready || path == nullptr) {
    return false;
  }

  const String parentDir = getParentDir(path);
  if (!parentDir.isEmpty() && parentDir != "/" && !ensureDir(parentDir.c_str())) {
    Serial.printf("[存储] 无法确保父目录存在: %s\n", parentDir.c_str());
    return false;
  }

  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.printf("[存储] 打开 %s 进行写入失败。\n", path);
    return false;
  }

  const size_t written = file.print(text);
  file.close();
  if (written != text.length()) {
    Serial.printf("[存储] 写入 %s 不完整，期望 %u，实际 %u。\n", path,
                  static_cast<unsigned>(text.length()), static_cast<unsigned>(written));
    return false;
  }

  return true;
}
