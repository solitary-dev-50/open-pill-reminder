# Open Pill Reminder Design Summary

Chinese original: [Open_Pill_Reminder_Design.md](./Open_Pill_Reminder_Design.md)

This file is the English release-facing summary for the current design documentation of Open Pill Reminder.

## Project Positioning

Open Pill Reminder is an offline-first local reminder box. It is not a medical device and does not provide medical advice, diagnosis, dosage guidance, medicine identification, or efficacy judgment.

The core goal of version 0.1 is:

1. Configure daily reminders locally
2. Trigger local reminders on the device
3. Confirm with a hardware button
4. Record confirmation results locally

## System Overview

Current hardware direction:

- ESP32-C3 compatible target
- ESP32-S3 bring-up and validation target
- I2C OLED
- Active buzzer
- `OK` button
- `SET/BACK` button
- External LED and onboard WS2812
- USB power

Current software direction:

- PlatformIO
- Arduino framework
- LittleFS
- Local AP mode web console
- Local JSON-based config and daily records

## Reminder Model

Current reminder model:

- Up to 8 reminders per day
- Each reminder contains `id`, `time`, and `enabled`
- All reminders repeat daily
- The user can add, edit, delete, enable, and disable reminders in the web console

Current repeat policy:

- If a reminder is not confirmed, it repeats after `repeat_interval_minutes`
- It stops after `max_repeat_count`

## Time Strategy

Version 0.1 does not require RTC hardware.

Current approach:

- The device starts in `TIME_NOT_SET` until time is synced
- The web console can sync browser-local time to the device
- Formal reminders are blocked until time becomes valid

This keeps `TimeService` replaceable so RTC can be added later without rewriting reminder or record logic.

## Data Storage

Current local storage:

- `/config.json` for device configuration
- `/records/YYYY-MM-DD.json` for daily records
- Web assets stored in LittleFS

## Web Console

Current web console responsibilities:

- Show current device status
- Show time sync state
- Manage reminders
- Configure repeat interval and maximum repeat count
- Trigger test actions for buzzer, LED, OLED, and reminder flow

## Firmware Structure

Main directories:

- `src/app/` for reminder, config, and record logic
- `src/drivers/` for hardware drivers
- `src/system/` for time and storage
- `src/web/` for local web server and APIs
- `data/` for web assets
- `include/` for board-level configuration

## Scope Boundary

Version 0.1 does not include:

- Cloud sync
- Mobile app
- Account system
- Remote notification
- Medicine name
- Dosage
- Day-of-week logic
- Multi-user support
- Medical decision logic

## Licensing

This documentation is covered by the repository documentation license structure. See:

- [../LICENSE-DOCS.en.md](../LICENSE-DOCS.en.md)
