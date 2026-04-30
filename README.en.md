# Open Pill Reminder / 小药记

Chinese version: [README.md](./README.md)

[![Build](https://github.com/solitary-dev-50/open-pill-reminder/actions/workflows/build.yml/badge.svg)](https://github.com/solitary-dev-50/open-pill-reminder/actions/workflows/build.yml)

Open Pill Reminder is an offline-first, minimalist, fully open source pill reminder box project.

This project is not a medical device. It does not provide medical advice and does not identify medicines, dosage, or efficacy. Version 0.1 focuses only on local reminders, local confirmation, and local records.

## Quick Start

- System overview: [DOC/Open_Pill_Reminder_Design.en.md](./DOC/Open_Pill_Reminder_Design.en.md)
- Hardware notes: [DOC/KB/02_hardware_plan.en.md](./DOC/KB/02_hardware_plan.en.md)
- PCB and schematic notes: [DOC/PCB/README.en.md](./DOC/PCB/README.en.md)
- Build locally: `pio run -e esp32-s3-devkitc-1`
- Build filesystem image: `pio run -e esp32-s3-devkitc-1 -t buildfs`

## Project Goal

Open Pill Reminder is designed to complete a simple and reliable local loop:

1. The user adds daily reminder times in the local web console.
2. The device alerts through OLED, buzzer, and LEDs when a reminder is due.
3. The user confirms with the `OK` button.
4. The device stores the confirmation result and timestamp locally.
5. If the reminder is not confirmed, it repeats according to the configured interval and repeat limit.

Core principles:

- Offline first
- No cloud dependency
- Local storage for configuration and records
- Firmware, web UI, documentation, schematics, and PCB files are all open source

## Open Source Scope

This repository is intended to be published as a complete open source project, including:

- Firmware source code
- Local web console
- Design documentation
- Hardware schematics
- PCB project files

Current repository contents:

- Firmware: [`src/`](./src)
- Board configuration: [`include/`](./include)
- Web assets: [`data/`](./data)
- Documentation: [`DOC/`](./DOC)
- PCB project files: [`DOC/PCB/`](./DOC/PCB)

Current PCB project file:

- [`DOC/PCB/小药记.eprj2`](./DOC/PCB/%E5%B0%8F%E8%8D%AF%E8%AE%B0.eprj2)

PCB and schematic preview:

- PCB notes page: [DOC/PCB/README.en.md](./DOC/PCB/README.en.md)

![PCB preview](<./DOC/PCB/小药记PCB.png>)

![PCB 3D preview](<./DOC/PCB/小药记 3D图.png>)

## Current Status

Current development baseline:

- `ESP32-S3` has been tested
- `ESP32-C3` board-level compatibility configuration is retained
- `PlatformIO + Arduino`
- `LittleFS`
- Local AP mode web console

Current working pieces:

- LittleFS initialization and configuration persistence
- `/config.json` read/write
- Local web console load and config save
- Browser-local time sync to device
- OLED status display
- Buzzer test
- External LED and onboard WS2812 status light
- `OK / SET` button events
- Local daily record storage
- Test reminder loop

## Version 0.1 Scope

V0.1 focuses on:

- Up to 8 reminders per day
- Each reminder includes `id / time / enabled`
- All reminders repeat daily
- The web console can add, edit, delete, enable, and disable reminders
- The user can set the repeat interval and maximum repeat count
- OLED, buzzer, and LED alert when a reminder is due
- `OK` confirmation is recorded with timestamp
- Configuration is stored in `LittleFS`
- Daily records are stored locally by date

Explicitly out of scope for V0.1:

- Cloud sync
- Mobile app
- Account system
- Remote notifications
- Medicine name
- Dosage management
- Day-of-week selection
- Multi-user support
- Voice
- Camera
- Battery management
- Medical features

## Hardware Direction

Current default bring-up board:

- `ESP32-S3-DevKitC-1`

Current default S3 pin mapping:

- OLED SDA: `GPIO8`
- OLED SCL: `GPIO9`
- Buzzer: `GPIO4`
- `OK` button: `GPIO5`
- `SET/BACK` button: `GPIO6`
- External LED: `GPIO7`
- Onboard WS2812: `GPIO38`

Button wiring:

- `GPIO5 -> tact switch -> GND`
- `GPIO6 -> tact switch -> GND`

Mechanical compatibility constraint:

- The Open Pill Reminder PCB is intended to fit a `Raspberry Pi 4B` case
- The PCB outline and mounting holes should match that enclosure direction
- The `USB-C` connector position should align with the enclosure opening
- The `OK` / `SET` button positions should align with enclosure button holes

This means the PCB is not only an electrical design target, but also a mechanical fit target.

Related drawings and previews:

- [DOC/PCB/README.en.md](./DOC/PCB/README.en.md)

## Time Strategy

V0.1 does not require an RTC.

The current time source is browser-local time sync:

- The user opens the local web console
- Clicks “Sync time from browser”
- The browser writes local date and time to the device

This means:

- Before sync, the device enters `TIME_NOT_SET`
- Formal reminders are blocked until time is valid
- The OLED shows `Time not set / Open Web Console`
- If RTC support is added later, only `TimeService` should change

## Local Storage

Current storage layout:

- Configuration: `/config.json`
- Daily records: `/records/YYYY-MM-DD.json`
- Web static assets: `LittleFS`

Notes:

- Auto-format on LittleFS mount failure has been disabled to avoid wiping configuration
- Running `uploadfs` rewrites the filesystem partition and can overwrite the current config

## Repository Structure

```text
src/
├─ app/       Reminder, configuration, and record logic
├─ drivers/   OLED, buzzer, button, and LED drivers
├─ system/    Time and storage services
└─ web/       Web server and API

include/      Board and application configuration
data/         Web console static assets
DOC/          Design docs, knowledge base, and PCB files
```

## Build and Flash

Development environment:

- PlatformIO
- Arduino framework

Default environment:

- `esp32-s3-devkitc-1`

Common commands:

```powershell
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-s3-devkitc-1
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-s3-devkitc-1 -t upload
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-s3-devkitc-1 -t buildfs
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-s3-devkitc-1 -t uploadfs
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio device monitor -b 115200 -p <your-port>
```

To build for `ESP32-C3`:

```powershell
$env:PLATFORMIO_CORE_DIR='.pio-core'; pio run -e esp32-c3-devkitm-1
```

## Documentation

Recommended entry points:

- System design: [DOC/Open_Pill_Reminder_Design.md](./DOC/Open_Pill_Reminder_Design.md)
- Knowledge base index: [DOC/KB/README.md](./DOC/KB/README.md)
- Disclaimer: [DISCLAIMER.en.md](./DISCLAIMER.en.md)
- Contributing guide: [CONTRIBUTING.en.md](./CONTRIBUTING.en.md)

## Licensing

This project uses a multi-license open source structure:

- Firmware source code, web console code, and scripts: `MIT License`
- Documentation: `CC BY-SA 4.0`
- Hardware schematics, PCB project files, and hardware design materials: `CERN-OHL-S-2.0`

License files:

- Code license: [LICENSE](./LICENSE)
- Documentation license notice: [LICENSE-DOCS.en.md](./LICENSE-DOCS.en.md)
- Hardware license notice: [LICENSE-HARDWARE.en.md](./LICENSE-HARDWARE.en.md)

## Disclaimer

This project is intended only for general pill reminders and confirmation records.

- It is not a medical device
- It does not provide medical advice
- It does not identify medicines, dosage, or efficacy
- It is not a substitute for doctors, pharmacists, caregivers, or professional medical systems

The project does not assume medical safety responsibility for problems caused by reminder failure, power loss, unsynced time, hardware damage, or missing user confirmation. These risks are the responsibility of the user.

## Contribution Areas

Contributions are welcome in:

- Firmware
- Web console frontend
- Hardware schematic and PCB improvements
- Enclosure and mechanical design
- Documentation
- Localization
- Testing and issue reports

## Commercial Use and Customization

Open Pill Reminder is a fully open source project. You may study, modify, prototype, manufacture, and build derivative work in accordance with the licenses used in this repository.

If you need hardware customization, PCB adaptation, firmware modification, enclosure work, localization, or small-batch implementation based on this project, you may contact the project maintainer.

For customization or collaboration, please open an issue in this repository.

This project is not a medical device and does not provide medical advice.
