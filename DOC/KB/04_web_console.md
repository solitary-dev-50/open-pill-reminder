# Web 控制台

## 目标

Web 控制台用于本地配置和调试，不做登录、不做云同步、不做复杂后台。

## 页面结构

### 1. 设备状态区

显示：

- 当前设备时间
- 当前设备状态
- 下一次提醒时间
- 今日已确认数量

### 2. 提醒列表区

显示每条提醒：

- `id`
- `time`
- `enabled`

支持操作：

- 新增
- 修改时间
- 启用/停用
- 删除

### 3. 全局设置区

字段：

- `repeat_interval_minutes`
- `max_repeat_count`

### 4. 设备测试区

支持：

- 测试蜂鸣器
- 测试 LED
- 测试 OLED
- 立即触发提醒

### 5. 今日记录区

显示：

- 日期
- 每条提醒的状态
- 确认时间

## 页面原则

- 简单
- 字大
- 适合手机浏览器
- 交互清楚
- 不做复杂动画

## API 方向

- `GET /api/status`
- `GET /api/config`
- `POST /api/config`
- `GET /api/reminders`
- `POST /api/reminders`
- `PUT /api/reminders/{id}`
- `DELETE /api/reminders/{id}`
- `GET /api/records/today`
- `POST /api/test/buzzer`
- `POST /api/test/led`
- `POST /api/test/oled`
- `POST /api/test/trigger`

## 交互流

1. 页面加载后读取状态、配置和今日记录
2. 用户修改提醒配置并保存
3. 页面提示保存成功或失败
4. 测试按钮调用对应测试接口
5. 今日记录区定时刷新或在关键操作后刷新

