# Changelog

## [1.0.0] - 2026-08-19

### Added
- ESP32-based dual motor automation.
- 2 HP and 6 HP motor control.
- Blynk IoT remote control and monitoring.
- Auto/Manual operation.
- Float switch monitoring.
- SCT current measurement using EmonLib.
- Over-current protection.
- Current-based dry-run protection.
- Maximum runtime protection.
- Startup and restart delays.
- 6 HP start/stop relay pulse control.
- Blynk fault/status notifications.
- Protection bypass controls for controlled testing/service.

### Code Quality
- Consolidated duplicate Blynk handlers.
- Consolidated duplicate motor timer declarations.
- Standardized dry-run delay constant.
- Consolidated sensor, startup and protection processing into one scheduled control routine.
- Removed obsolete debug-only duplicate sections.
- Added professional documentation and test guidance.
