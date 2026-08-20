# Hardware Wiring Reference

## ESP32 GPIO Allocation

| GPIO | Signal | Type |
|---:|---|---|
| 17 | 2 HP relay | Output |
| 27 | 6 HP START relay | Output |
| 13 | 6 HP STOP relay | Output |
| 32 | Tank LOW float | Input |
| 33 | Tank FULL float | Input |
| 26 | Sump FULL float | Input |
| 25 | Sump DRY float | Input |
| 34 | 2 HP SCT current | ADC input |
| 35 | 6 HP SCT current | ADC input |

## Relay Logic

The firmware uses active-LOW relay outputs:

- `LOW` = relay energized
- `HIGH` = relay released

The 6 HP motor uses separate START and STOP pulse outputs.

## Float Inputs

Float inputs are configured using:

```cpp
INPUT_PULLUP
```

Therefore the firmware interprets:

```text
LOW  = switch active
HIGH = switch inactive
```

Verify the actual field wiring and float contact configuration before commissioning.

## Current Sensors

The firmware uses EmonLib:

```cpp
emon2HP.current(CURRENT_2HP_PIN, SCT_2HP_CALIBRATION);
emon6HP.current(CURRENT_6HP_PIN, SCT_6HP_CALIBRATION);
```

The calibration constants must be verified against the actual SCT sensor and installation.

## Mains Safety

Do not connect mains voltage to ESP32 GPIO pins.

Use an isolated, correctly rated interface and a properly designed contactor/control circuit. Final wiring must be checked by a qualified electrical professional.
