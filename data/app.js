// SPDX-License-Identifier: MIT

const state = {
  language: localStorage.getItem("open-pill-language") || "zh-CN",
  config: {
    repeat_interval_minutes: 5,
    max_repeat_count: 3,
    reminders: [],
  },
};

const I18N = {
  "zh-CN": {
    app_title: "小药记",
    app_subtitle: "V0.1 本地网页控制台",
    language: "语言 / Language",
    status_title: "设备状态",
    current_time: "当前时间",
    current_date: "当前日期",
    reminder_state: "提醒状态",
    time_status: "时间状态",
    next_reminder: "下次提醒",
    confirmed_summary: "已确认 / 总数",
    time_source: "时间来源",
    ap_ssid: "热点名称",
    device_ip: "设备地址",
    sync_time_btn: "从浏览器同步时间",
    reminder_config_title: "提醒配置",
    add_reminder: "添加提醒",
    reminder_help: "每天最多 8 个提醒，时间格式为 HH:MM。",
    global_settings_title: "全局设置",
    repeat_interval_label: "重复提醒间隔（分钟）",
    repeat_interval_help: "未确认时，隔多少分钟再次提醒。",
    max_repeat_label: "最大重复提醒次数",
    max_repeat_help: "首次提醒后，最多再重复提醒多少次。",
    save_config: "保存配置",
    reload: "重新读取",
    device_test_title: "设备测试",
    test_buzzer: "测试蜂鸣器",
    test_led: "测试指示灯",
    test_oled: "测试 OLED",
    test_trigger: "立即触发提醒",
    today_records_title: "今日日志",
    messages_title: "消息",
    waiting_message: "等待操作...",
    enabled: "启用",
    delete: "删除",
    browser_sync: "浏览器同步",
    synced: "已同步",
    not_synced: "未同步",
    no_reminder: "无",
    message_json_error: "JSON 解析失败",
    message_request_failed: "请求失败",
    message_config_saved: "配置已保存。",
    message_select_time: "请先选择提醒时间。",
    message_max_reminders: "每天最多 8 个提醒。",
    message_action_done: "已执行测试",
    message_reloaded: "已重新读取设备状态和配置。",
    message_time_synced: "设备时间已同步。",
    message_console_connected: "控制台已连接。",
    state_time_not_set: "时间未设置",
    state_idle: "待机",
    state_alerting: "提醒中",
    state_test: "测试提醒",
    state_unknown: "未知",
  },
  en: {
    app_title: "Open Pill Reminder",
    app_subtitle: "V0.1 Local Web Console",
    language: "Language / 语言",
    status_title: "Device Status",
    current_time: "Current Time",
    current_date: "Current Date",
    reminder_state: "Reminder State",
    time_status: "Time Status",
    next_reminder: "Next Reminder",
    confirmed_summary: "Confirmed / Total",
    time_source: "Time Source",
    ap_ssid: "AP SSID",
    device_ip: "Device IP",
    sync_time_btn: "Sync Time From Browser",
    reminder_config_title: "Reminder Configuration",
    add_reminder: "Add Reminder",
    reminder_help: "Up to 8 reminders per day. Time format: HH:MM.",
    global_settings_title: "Global Settings",
    repeat_interval_label: "Repeat Interval (minutes)",
    repeat_interval_help: "If not confirmed, repeat again after this many minutes.",
    max_repeat_label: "Max Repeat Count",
    max_repeat_help: "After the first alert, repeat at most this many more times.",
    save_config: "Save Configuration",
    reload: "Reload",
    device_test_title: "Device Tests",
    test_buzzer: "Test Buzzer",
    test_led: "Test LED",
    test_oled: "Test OLED",
    test_trigger: "Trigger Reminder Now",
    today_records_title: "Today's Records",
    messages_title: "Messages",
    waiting_message: "Waiting for action...",
    enabled: "Enabled",
    delete: "Delete",
    browser_sync: "Browser Sync",
    synced: "Synced",
    not_synced: "Not synced",
    no_reminder: "None",
    message_json_error: "JSON parse failed",
    message_request_failed: "Request failed",
    message_config_saved: "Configuration saved.",
    message_select_time: "Please select a reminder time first.",
    message_max_reminders: "Up to 8 reminders per day.",
    message_action_done: "Test action executed",
    message_reloaded: "Device status and configuration reloaded.",
    message_time_synced: "Device time synced.",
    message_console_connected: "Console connected.",
    state_time_not_set: "Time not set",
    state_idle: "Idle",
    state_alerting: "Alerting",
    state_test: "Test Alert",
    state_unknown: "Unknown",
  },
};

function t(key) {
  return I18N[state.language]?.[key] ?? I18N["zh-CN"][key] ?? key;
}

function message(text) {
  document.getElementById("message-box").textContent = text;
}

function translateBackendState(value) {
  const map = {
    "时间未设置": t("state_time_not_set"),
    "待机": t("state_idle"),
    "提醒中": t("state_alerting"),
    "测试提醒": t("state_test"),
    "未知": t("state_unknown"),
  };
  return map[value] || value || "--";
}

function translateTimeStatus(value) {
  const map = {
    已同步: t("synced"),
    未同步: t("not_synced"),
  };
  return map[value] || value || "--";
}

function translateTimeSource(value) {
  if (value === "browser_sync") {
    return t("browser_sync");
  }
  return value || "--";
}

function applyLanguage() {
  document.documentElement.lang = state.language;
  document.title =
    state.language === "en" ? "Open Pill Reminder" : "小药记 / Open Pill Reminder";

  document.querySelectorAll("[data-i18n]").forEach((node) => {
    node.textContent = t(node.dataset.i18n);
  });

  document.getElementById("language-select").value = state.language;
  renderConfig();
  message(t("waiting_message"));
}

async function fetchJson(url, options = {}) {
  const response = await fetch(url, options);
  const text = await response.text();
  let data = {};

  try {
    data = text ? JSON.parse(text) : {};
  } catch (error) {
    throw new Error(`${t("message_json_error")}: ${text}`);
  }

  if (!response.ok) {
    throw new Error(data.error || `${t("message_request_failed")}: ${response.status}`);
  }

  return data;
}

function renderStatus(status) {
  document.getElementById("current-time").textContent = status.current_time || "--";
  document.getElementById("current-date").textContent = status.current_date || "--";
  document.getElementById("current-state").textContent = translateBackendState(status.state);
  document.getElementById("time-status").textContent = translateTimeStatus(status.time_status);
  document.getElementById("next-reminder").textContent = status.next_reminder || t("no_reminder");
  document.getElementById("confirmed-summary").textContent =
    `${status.confirmed_count ?? 0} / ${status.total_count ?? 0}`;
  document.getElementById("time-source").textContent = translateTimeSource(status.time_source);
  document.getElementById("ap-ssid").textContent = status.ap_ssid || "--";
  document.getElementById("device-ip").textContent = status.ip || "--";
}

function renderConfig() {
  const repeatInput = document.getElementById("repeat-interval");
  const maxRepeatInput = document.getElementById("max-repeat-count");
  if (repeatInput) {
    repeatInput.value = state.config.repeat_interval_minutes;
  }
  if (maxRepeatInput) {
    maxRepeatInput.value = state.config.max_repeat_count;
  }

  const list = document.getElementById("reminder-list");
  if (!list) {
    return;
  }
  list.innerHTML = "";

  state.config.reminders.forEach((reminder, index) => {
    const row = document.createElement("div");
    row.className = "reminder-row";

    const timeInput = document.createElement("input");
    timeInput.type = "time";
    timeInput.value = reminder.time;
    timeInput.addEventListener("change", () => {
      state.config.reminders[index].time = timeInput.value;
    });

    const enabledLabel = document.createElement("label");
    enabledLabel.className = "toggle";
    const enabledCheckbox = document.createElement("input");
    enabledCheckbox.type = "checkbox";
    enabledCheckbox.checked = reminder.enabled;
    enabledCheckbox.addEventListener("change", () => {
      state.config.reminders[index].enabled = enabledCheckbox.checked;
    });
    enabledLabel.append(enabledCheckbox, document.createTextNode(` ${t("enabled")}`));

    const idView = document.createElement("span");
    idView.textContent = reminder.id;

    const deleteBtn = document.createElement("button");
    deleteBtn.type = "button";
    deleteBtn.className = "secondary";
    deleteBtn.textContent = t("delete");
    deleteBtn.addEventListener("click", () => {
      state.config.reminders.splice(index, 1);
      renderConfig();
    });

    row.append(timeInput, enabledLabel, idView, deleteBtn);
    list.appendChild(row);
  });
}

function collectConfigFromForm() {
  state.config.repeat_interval_minutes = Number(
    document.getElementById("repeat-interval").value || 5,
  );
  state.config.max_repeat_count = Number(
    document.getElementById("max-repeat-count").value || 3,
  );
  return state.config;
}

async function loadStatus() {
  const status = await fetchJson("/api/status");
  renderStatus(status);
}

async function loadConfig() {
  const config = await fetchJson("/api/config");
  state.config = config;
  renderConfig();
}

async function loadTodayRecords() {
  const records = await fetchJson("/api/records/today");
  document.getElementById("today-records").textContent = JSON.stringify(records, null, 2);
}

async function syncBrowserTime() {
  const now = new Date();
  await fetchJson("/api/time/sync", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      epoch: Math.floor(now.getTime() / 1000),
      timezone_offset_minutes: -now.getTimezoneOffset(),
    }),
  });
}

async function saveConfig() {
  const payload = collectConfigFromForm();
  await fetchJson("/api/config", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  });

  message(t("message_config_saved"));
  await loadStatus();
  await loadConfig();
  await loadTodayRecords();
}

function addReminder() {
  const input = document.getElementById("new-reminder-time");
  const time = input.value;
  if (!time) {
    message(t("message_select_time"));
    return;
  }

  if (state.config.reminders.length >= 8) {
    message(t("message_max_reminders"));
    return;
  }

  state.config.reminders.push({
    id: `r${Date.now()}`,
    time,
    enabled: true,
  });

  input.value = "";
  renderConfig();
}

async function runTest(action) {
  await fetchJson(`/api/test/${action}`, { method: "POST" });
  message(`${t("message_action_done")}: ${action}`);
}

function bindEvents() {
  document.getElementById("language-select").addEventListener("change", (event) => {
    state.language = event.target.value;
    localStorage.setItem("open-pill-language", state.language);
    applyLanguage();
  });

  document.getElementById("add-reminder-btn").addEventListener("click", addReminder);
  document.getElementById("save-config-btn").addEventListener("click", saveConfig);
  document.getElementById("reload-btn").addEventListener("click", async () => {
    await loadStatus();
    await loadConfig();
    await loadTodayRecords();
    message(t("message_reloaded"));
  });

  document.getElementById("sync-time-btn").addEventListener("click", async () => {
    try {
      await syncBrowserTime();
      await loadStatus();
      await loadTodayRecords();
      message(t("message_time_synced"));
    } catch (error) {
      message(error.message);
    }
  });

  document.querySelectorAll("[data-action]").forEach((button) => {
    button.addEventListener("click", async () => {
      try {
        await runTest(button.dataset.action);
      } catch (error) {
        message(error.message);
      }
    });
  });
}

async function init() {
  applyLanguage();
  bindEvents();
  await syncBrowserTime();
  await loadStatus();
  await loadConfig();
  await loadTodayRecords();
  message(t("message_console_connected"));

  window.setInterval(() => {
    Promise.all([loadStatus(), loadTodayRecords()]).catch((error) => message(error.message));
  }, 3000);
}

init().catch((error) => {
  message(error.message);
});
