# Smart Water Pump Automation System using ESP32 & Blynk IoT

![ESP32](https://img.shields.io/badge/ESP32-DevKit%20V1-blue)
![Arduino](https://img.shields.io/badge/Arduino-IDE-00979D)
![Blynk](https://img.shields.io/badge/Blynk-IoT-23C48E)
![C%2B%2B](https://img.shields.io/badge/Language-Embedded%20C%2B%2B-orange)
![Version](https://img.shields.io/badge/Version-1.0-success)
![License](https://img.shields.io/badge/License-MIT-yellow)

## Overview

The **Smart Water Pump Automation System** is an ESP32-based IoT controller for automated and remote monitoring of two water pumps:

- 2 HP water pump
- 6 HP water pump

The controller combines water-level sensing, AC-current monitoring, motor protection logic, relay control, Wi-Fi connectivity and the Blynk IoT platform.

The firmware supports **Automatic Mode** and **Manual Mode** and provides protection against conditions such as over-current, dry-run, tank-full, sump-full/sump-dry and excessive continuous runtime.

> **Important:** This repository contains the controller firmware and documentation. The ESP32 must not directly switch a mains motor. Use appropriately rated contactors, overload protection, isolation and an electrician-approved control panel.

---

## Key Features

### Motor Control
- 2 HP motor Start/Stop
- 6 HP motor Start/Stop
- Automatic operation
- Manual operation from Blynk
- Momentary relay pulse control for 6 HP Start/Stop
- Restart delay protection

### Water-Level Automation
- Overhead tank LOW indication
- Overhead tank FULL indication
- Sump DRY indication
- Sump FULL indication
- Automatic motor decisions based on float status

### Motor Protection
- Over-current protection
- Current-based dry-run protection
- Maximum runtime protection
- Startup protection delay
- Restart delay
- Float protection
- Blynk-selectable protection bypass controls for testing/service

### IoT Monitoring
- Blynk dashboard
- Live 2 HP current
- Live 6 HP current
- Motor running status
- Water-level indicators
- Last fault/status
- Blynk event notifications
- Auto/Manual mode indication

### Signal Processing
- SCT current sensors
- EmonLib RMS current calculation
- Low-current noise suppression
- Simple low-pass filtering

---

## System Architecture

```text
                   ┌─────────────────────┐
                   │   Blynk Cloud / App  │
                   └──────────┬──────────┘
                              │ Wi-Fi
                              │
                   ┌──────────▼──────────┐
                   │    ESP32 DevKit V1  │
                   │                     │
                   │  Control Logic      │
                   │  Protection Logic   │
                   │  IoT Communication  │
                   └───────┬───────┬─────┘
                           │       │
              ┌────────────┘       └────────────┐
              │                                 │
       ┌──────▼──────┐                   ┌──────▼──────┐
       │ Float Inputs│                   │ SCT Sensors │
       │ Tank / Sump │                   │ 2 HP / 6 HP │
       └─────────────┘                   └─────────────┘
              │                                 │
              └────────────┬────────────────────┘
                           │
                    ┌──────▼──────┐
                    │ Relay /     │
                    │ Contactor   │
                    │ Interface   │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │ Water Pumps │
                    │ 2 HP / 6 HP │
                    └─────────────┘
```

---

## Operating Logic

### 2 HP Pump

In Automatic Mode:

```text
Tank LOW
   ↓
Start 2 HP
   ↓
Monitor current / protection
   ↓
Tank FULL → Stop
```

Additional trips:

```text
Over Current       → Stop
Dry Run Current    → Stop
Sump Dry           → Stop
Maximum Runtime    → Stop
```

### 6 HP Pump

In Automatic Mode:

```text
Sump DRY + Sump not FULL
   ↓
Start 6 HP
   ↓
Monitor current / protection
   ↓
Sump FULL → Stop
```

Additional trips:

```text
Over Current       → Stop
Dry Run Current    → Stop
Maximum Runtime    → Stop
```

---

## Protection Timing

| Parameter | Firmware Value |
|---|---:|
| Startup protection delay | 10 s |
| Over-current confirmation delay | 3 s |
| Dry-run confirmation delay | 10 s |
| Restart delay | 5 s |
| 6 HP relay pulse | 1 s |
| Current Blynk update | 5 s |
| Runtime counter update | 60 s |
| Test-mode maximum runtime | 50 s |
| Normal maximum runtime | 15 min |

The runtime limit is compile-time selectable through:

```cpp
#define TEST_MODE 0
```

Use:

```cpp
#define TEST_MODE 1
```

for controlled bench testing.

---

## Current Protection

The firmware uses EmonLib to calculate RMS current from the SCT sensors.

Current processing includes:

1. RMS calculation
2. Low-level noise suppression
3. Low-pass filtering
4. Over-current threshold comparison
5. Time confirmation before trip

Configured values in the firmware:

```cpp
CURRENT_LIMIT_2HP = 13.0;
CURRENT_LIMIT_6HP = 13.0;
DRY_RUN_CURRENT   = 1.0;
```

**These are project configuration values and must be verified against the actual motor nameplate, supply voltage, sensor calibration and protection requirements before field deployment.**

---

## Blynk Virtual Pin Map

| Virtual Pin | Function |
|---|---|
| V1 | 2 HP Start |
| V2 | 6 HP Start |
| V3 | 2 HP Current |
| V4 | 6 HP Current |
| V5 | 2 HP Running Status |
| V6 | 6 HP Running Status |
| V7 | Tank FULL |
| V8 | Tank LOW |
| V9 | Sump FULL |
| V10 | Sump DRY |
| V11 | 2 HP Stop |
| V12 | 6 HP Stop |
| V15 | Last Fault / Status |
| V19 | Auto / Manual |
| V20 | Auto Mode LED |
| V21 | Mode Text |
| V22 | 2 HP Runtime |
| V23 | 6 HP Runtime |
| V25 | Current Monitoring Enable |
| V26 | Float Protection Bypass |
| V27 | Over-current Protection Bypass |
| V28 | Dry-run Protection Bypass |

---

## ESP32 Pin Map

| ESP32 GPIO | Function |
|---:|---|
| GPIO 17 | 2 HP relay |
| GPIO 27 | 6 HP START relay |
| GPIO 13 | 6 HP STOP relay |
| GPIO 32 | Tank LOW float |
| GPIO 33 | Tank FULL float |
| GPIO 26 | Sump FULL float |
| GPIO 25 | Sump DRY float |
| GPIO 34 | 2 HP SCT current input |
| GPIO 35 | 6 HP SCT current input |

GPIO 34 and GPIO 35 are input-only ESP32 pins and are used only for current sensing.

---

## Hardware

Typical system hardware:

- ESP32 DevKit V1
- SCT current sensors ×2
- Float switches ×4
- Relay interface
- Motor contactors
- Appropriate overload/short-circuit protection
- Isolated low-voltage power supply
- Wi-Fi router
- 2 HP pump
- 6 HP pump

### Electrical Safety

The ESP32 and its relay interface are **control electronics only**. Motor power circuits must be designed separately using correctly rated:

- Contactors
- MCB/MCCB/fuses
- Overload relays
- Earthing
- Isolation
- Control transformers/SMPS where applicable
- Enclosures and terminal blocks

Do not connect mains voltage directly to ESP32 GPIO pins.

---

## Software Requirements

### Arduino IDE

Install the ESP32 board package and select an appropriate ESP32 DevKit board.

### Required Libraries

- ESP32 Arduino core
- Blynk
- EmonLib

See:

`libraries/Required_Libraries.md`

---

## Configuration

Before compiling, update the credentials in the firmware:

```cpp
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Motor Automation"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";
```

**Never commit real Wi-Fi passwords or Blynk Auth Tokens to a public GitHub repository.**

---

## Test Procedure

Recommended bench-testing sequence:

1. Program the ESP32 without connecting motor power.
2. Verify Serial Monitor at `115200 baud`.
3. Verify Blynk connection.
4. Verify Auto/Manual switch.
5. Verify float indications.
6. Verify Start/Stop relay outputs with an isolated test load.
7. Verify 6 HP Start pulse.
8. Verify 6 HP Stop pulse.
9. Verify current sensor readings.
10. Enable Test Mode and verify runtime trip.
11. Test over-current logic with a safe simulated/current-source condition.
12. Test dry-run logic without connecting a live motor.
13. Verify Blynk notifications.
14. Only after all bench tests pass should the control panel be connected to the actual motor system.

See `docs/TEST_PLAN.md`.

---

## Project Structure

```text
Smart-Water-Pump-Automation-ESP32/
│
├── SmartWaterPumpAutomation.ino
├── README.md
├── LICENSE
├── CHANGELOG.md
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── .gitignore
│
├── docs/
│   ├── BLYNK_VIRTUAL_PINS.md
│   ├── HARDWARE_WIRING.md
│   └── TEST_PLAN.md
│
├── libraries/
│   └── Required_Libraries.md
│
└── images/
    └── README.md
```

---

## Future Development

Planned enhancements may include:

- Energy measurement
- Power factor monitoring
- Flow sensor
- Water consumption logging
- Web dashboard
- MQTT support
- OTA firmware updates
- GSM/4G backup communication
- Historical cloud data
- Advanced alarm management
- PCB revision with improved protection and isolation

---

## Developer

**Narala Heshma Sree**  
B.Tech – Electrical & Electronics Engineering  
SRM Institute of Science and Technology

Areas demonstrated:

- Embedded C++
- ESP32
- IoT
- Blynk
- Electrical automation
- Motor protection
- Sensor interfacing
- Relay control
- Industrial control concepts

---

## Disclaimer

This project is intended for educational, prototype and engineering-development purposes.

The firmware alone does not constitute a certified motor protection system. Final installation must comply with applicable electrical standards, equipment ratings, protection coordination and site safety procedures.

---

## License

Released under the MIT License. See `LICENSE`.
