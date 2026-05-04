# 小药记 / Open Pill Reminder

English version: [README.md](./README.md) | English docs index: [DOC/KB/README.en.md](./DOC/KB/README.en.md)

[![Build](https://github.com/solitary-dev-50/open-pill-reminder/actions/workflows/build.yml/badge.svg)](https://github.com/solitary-dev-50/open-pill-reminder/actions/workflows/build.yml)

一个离线优先、极简、完整开源的智能服药提醒盒项目。

本项目不是医疗设备，不提供医疗建议，不判断药品种类、剂量或疗效。第一版只做本地提醒、用户确认和本地记录。

## 快速开始

- 看项目概览：[DOC/Open_Pill_Reminder_Design.md](./DOC/Open_Pill_Reminder_Design.md)
- 看硬件方案：[DOC/KB/02_hardware_plan.md](./DOC/KB/02_hardware_plan.md)
- 看 PCB 和原理图：[DOC/PCB/README.md](./DOC/PCB/README.md)
- 本地编译：`pio run -e esp32-s3-devkitc-1`
- 构建文件系统：`pio run -e esp32-s3-devkitc-1 -t buildfs`

## 项目目标

小药记希望跑通一个清晰、可靠、可复现的本地闭环：

1. 用户在本地 Web 控制台添加每天的提醒时间
2. 到点后设备通过 OLED、蜂鸣器、指示灯发出提醒
3. 用户按 `OK` 键确认
4. 设备把确认结果和确认时间保存到本地
5. 未确认时按设定间隔重复提醒，直到达到最大重复次数

核心原则：

- 离线优先
- 不依赖云端
- 本地保存配置和记录
- 固件、Web、文档、原理图、PCB 一并开源

## 开源范围

这个仓库按完整开源项目组织，目标包括：

- 固件源码
- 本地 Web 控制台
- 设计文档
- 硬件原理图
- PCB 工程文件

当前仓库中的对应内容：

- 固件代码：[`src/`](./src)
- 板级配置：[`include/`](./include)
- Web 页面资源：[`data/`](./data)
- 设计文档：[`DOC/`](./DOC)
- PCB 工程文件：[`DOC/PCB/`](./DOC/PCB)

当前 `PCB` 目录中已包含工程文件：

- [`DOC/PCB/小药记.eprj2`](./DOC/PCB/%E5%B0%8F%E8%8D%AF%E8%AE%B0.eprj2)

PCB 与原理图预览：

- PCB 说明页：[DOC/PCB/README.md](./DOC/PCB/README.md)

![PCB 预览](<./DOC/PCB/小药记PCB.png>)

![PCB 3D 预览](<./DOC/PCB/小药记 3D图.png>)

## 当前状态

当前版本基于：

- `ESP32-S3` 已完成联调
- `ESP32-C3` 保留兼容板级配置
- `PlatformIO + Arduino`
- `LittleFS`
- 本地 AP 模式 Web 控制台

当前已经打通的能力：

- LittleFS 初始化与配置保存
- `/config.json` 配置读写
- 本地 Web 控制台打开与配置保存
- 浏览器本地时间同步到设备
- OLED 状态显示
- 蜂鸣器测试
- 外接 LED 与板载 WS2812 指示灯
- `OK / SET` 按键事件
- 今日记录本地保存
- 测试提醒闭环

## 第一版范围

V0.1 第一版聚焦这些能力：

- 每天最多支持 8 个提醒
- 每个提醒包含 `id / time / enabled`
- 所有提醒每天重复
- 用户可在 Web 页面添加、修改、删除、启用、停用提醒
- 可设置重复提醒间隔和最大重复次数
- 提醒时触发 OLED、蜂鸣器、LED
- 用户按 `OK` 键后记录确认时间
- 配置保存在 `LittleFS`
- 今日记录按日期保存到本地

第一版明确不做：

- 云同步
- 手机 App
- 账号系统
- 远程通知
- 药品名称
- 剂量管理
- 星期选择
- 多用户
- 语音
- 摄像头
- 电池管理
- 医疗功能

## 硬件方向

当前默认联调板为：

- `ESP32-S3-DevKitC-1`

当前 S3 默认引脚：

- OLED SDA: `GPIO8`
- OLED SCL: `GPIO9`
- 蜂鸣器: `GPIO4`
- `OK` 按键: `GPIO5`
- `SET/BACK` 按键: `GPIO6`
- 外接 LED: `GPIO7`
- 板载 WS2812: `GPIO38`

按键接法：

- `GPIO5 -> 轻触开关 -> GND`
- `GPIO6 -> 轻触开关 -> GND`

结构兼容约束：

- 小药记 PCB 目标为完美适配树莓派 `Raspberry Pi 4B` 外壳
- PCB 外形尺寸和安装孔位需要与该外壳方案匹配
- `USB-C` 接口开孔位置需要与外壳对应
- `OK` / `SET` 按键孔位需要与外壳对应

这意味着 PCB 设计不仅是电气设计，还必须同时满足外壳装配与开孔约束。

相关图纸与预览见：

- [DOC/PCB/README.md](./DOC/PCB/README.md)

## 时间策略

V0.1 不强制依赖 `RTC`。

当前时间来源是浏览器本地时间同步：

- 用户打开本地 Web 控制台
- 点击“从浏览器同步时间”
- 浏览器把本地日期时间写入设备

这意味着：

- 未同步前设备进入 `TIME_NOT_SET`
- 未同步时不触发正式提醒
- OLED 会提示 `Time not set / Open Web Console`
- 后续如果增加 `RTC`，只需要替换 `TimeService`，不需要重写提醒逻辑

## 本地存储

当前数据保存策略：

- 配置：`/config.json`
- 今日记录：`/records/YYYY-MM-DD.json`
- Web 静态页面：`LittleFS`

说明：

- 当前版本已经禁用 `LittleFS` 挂载失败时自动格式化，避免误清空配置
- 如果执行 `uploadfs`，会重写文件系统分区，现有配置可能被覆盖

## 仓库结构

```text
src/
├─ app/       业务逻辑：提醒、配置、记录
├─ drivers/   硬件驱动：OLED、蜂鸣器、按键、指示灯
├─ system/    系统模块：时间、存储
└─ web/       WebServer 与 API

include/      板级配置与全局配置
data/         Web 控制台静态资源
DOC/          设计文档、知识库、PCB 工程文件
```

## 构建与烧录

开发环境：

- PlatformIO
- Arduino framework

当前默认环境：

- `esp32-s3-devkitc-1`

常用命令：

```powershell
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-s3-devkitc-1
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-s3-devkitc-1 -t upload
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-s3-devkitc-1 -t buildfs
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-s3-devkitc-1 -t uploadfs
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio device monitor -b 115200 -p <your-port>
```

如需切换到 `ESP32-C3`：

```powershell
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-c3-devkitm-1
```

## 文档入口

建议先看这些文档：

- 总体设计：[DOC/Open_Pill_Reminder_Design.md](./DOC/Open_Pill_Reminder_Design.md)
- 知识库索引：[DOC/KB/README.md](./DOC/KB/README.md)
- 项目定位：[DOC/KB/00_project_positioning.md](./DOC/KB/00_project_positioning.md)
- 第一版范围：[DOC/KB/01_v1_scope.md](./DOC/KB/01_v1_scope.md)
- 硬件方案：[DOC/KB/02_hardware_plan.md](./DOC/KB/02_hardware_plan.md)
- 固件逻辑：[DOC/KB/03_firmware_logic.md](./DOC/KB/03_firmware_logic.md)
- Web 控制台：[DOC/KB/04_web_console.md](./DOC/KB/04_web_console.md)
- 免责声明：[DISCLAIMER.md](./DISCLAIMER.md)
- 贡献指南：[CONTRIBUTING.md](./CONTRIBUTING.md)

## 许可证

本项目采用多许可证开源结构：

- 固件源码、Web 控制台代码、脚本：`MIT License`
- 文档：`CC BY-SA 4.0`
- 硬件原理图、PCB 工程文件、硬件设计资料：`CERN-OHL-S-2.0`

对应文件：

- 代码许可证：[LICENSE](./LICENSE)
- 文档许可证说明：[LICENSE-DOCS](./LICENSE-DOCS)
- 硬件许可证说明：[LICENSE-HARDWARE](./LICENSE-HARDWARE)

## 免责声明

本项目仅用于通用服药提醒和确认记录。

- 本项目不是医疗设备
- 本项目不提供医疗建议
- 本项目不判断药品种类、剂量或疗效
- 本项目不替代医生、药师、护理人员或专业医疗系统

提醒失败、断电、时间未同步、硬件损坏或用户未确认造成的问题，项目不承担医疗保障责任，相关风险由使用者自行承担。

详细说明见：

- 中文：[DISCLAIMER.md](./DISCLAIMER.md)
- English: [DISCLAIMER.en.md](./DISCLAIMER.en.md)

## 贡献方向

欢迎以下方向的贡献：

- 固件开发
- Web 控制台前端
- 硬件原理图与 PCB 改进
- 外壳与结构设计
- 文档整理
- 多语言支持
- 测试与问题反馈

提交 PR 前，请阅读：

- 中文：[CONTRIBUTING.md](./CONTRIBUTING.md)
- English: [CONTRIBUTING.en.md](./CONTRIBUTING.en.md)

## 商业使用与定制

小药记是一个完整开源项目。你可以根据本仓库许可证学习、修改、打样、生产和二次开发。

如果你需要基于本项目做硬件定制、PCB 适配、固件修改、外壳结构、多语言本地化或小批量落地，可以联系项目维护者。

如需定制或协作，可以在本仓库提交 Issue 联系维护者。

本项目不是医疗设备，不提供医疗建议。
