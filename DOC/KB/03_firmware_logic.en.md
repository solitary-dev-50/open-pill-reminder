# Firmware Logic

Chinese original: [03_firmware_logic.md](./03_firmware_logic.md)

Current firmware structure is split by responsibility:

- `ReminderManager` for scheduling and reminder state
- `ConfigManager` for loading and saving configuration
- `RecordManager` for daily records
- `TimeService` for time validity and browser sync
- `Storage` for filesystem access
- `WebServer` for the local UI and APIs

Current design rules:

- Time sync logic stays inside `TimeService`
- Reminder logic does not assume RTC
- Records use synced local date and time
- The main loop should remain thin

The project intentionally avoids putting all logic into `main.cpp`.
