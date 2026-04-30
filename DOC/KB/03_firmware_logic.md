# 固件逻辑

## 第一版固件目标

固件只围绕一个闭环工作：

`调度提醒 -> 触发提醒 -> 等待确认 -> 记录结果`

## 建议模块

- `main`：启动与主循环
- `ReminderManager`：提醒调度与状态机
- `ConfigManager`：配置读写与合法性校验
- `RecordManager`：今日记录维护
- `DisplayManager`：OLED 内容显示
- `BuzzerDriver`：蜂鸣器控制
- `LedDriver`：LED 控制
- `ButtonManager`：按键扫描与消抖
- `WebServer`：本地网页与 API
- `Storage`：本地文件系统封装
- `TimeService`：当前时间和日期处理

## 状态机

建议以“单个提醒的当天生命周期”为核心，而不是只用一个全局大状态。

### 状态

- `waiting_today`
- `alerting`
- `waiting_repeat`
- `confirmed`
- `expired`
- `disabled`

### 状态含义

- `waiting_today`：等待今天到点
- `alerting`：正在声光屏提醒
- `waiting_repeat`：等待下一轮重复提醒
- `confirmed`：用户已确认
- `expired`：达到最大重复次数仍未确认
- `disabled`：该提醒当前未启用

### 状态迁移

`waiting_today -> alerting -> confirmed`

`waiting_today -> alerting -> waiting_repeat -> alerting`

`alerting -> expired`

`disabled` 不参与调度。

## 配置数据

建议保留：

```json
{
  "repeat_interval_minutes": 5,
  "max_repeat_count": 3,
  "reminders": [
    {
      "id": "r001",
      "time": "08:00",
      "enabled": true
    }
  ]
}
```

## 今日记录

建议保留：

```json
{
  "date": "2026-04-27",
  "items": [
    {
      "reminder_id": "r001",
      "scheduled_time": "08:00",
      "status": "confirmed",
      "confirmed_at": "08:03"
    }
  ]
}
```

## 主循环职责

- 处理按键事件
- 检查当前时间
- 推进提醒状态机
- 更新 OLED
- 处理 Web 请求
- 检查日期切换

## 边界建议

- 配置数量超过 8 时拒绝保存
- 时间格式非法时拒绝保存
- 日期切换时重建当天提醒实例
- 存储失败时必须返回错误，不允许静默失败

