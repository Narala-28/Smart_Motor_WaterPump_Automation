# Test Plan

## Stage 1 — Firmware Bench Test

- [ ] Upload firmware with motors disconnected.
- [ ] Confirm ESP32 boots correctly.
- [ ] Confirm Serial Monitor at 115200 baud.
- [ ] Confirm Blynk connection.
- [ ] Confirm Auto/Manual status.
- [ ] Confirm Blynk Start/Stop push buttons.
- [ ] Confirm relay outputs with an isolated test load.

## Stage 2 — Float Inputs

- [ ] Test Tank LOW.
- [ ] Test Tank FULL.
- [ ] Test Sump DRY.
- [ ] Test Sump FULL.
- [ ] Confirm corresponding Blynk indicators.
- [ ] Confirm automatic start/stop decisions.

## Stage 3 — Current Measurement

- [ ] Verify 2 HP SCT reading.
- [ ] Verify 6 HP SCT reading.
- [ ] Verify sensor calibration.
- [ ] Verify low-current noise suppression.
- [ ] Verify Blynk current updates.

## Stage 4 — Protection Tests

- [ ] Test over-current confirmation delay safely.
- [ ] Test dry-run confirmation delay safely.
- [ ] Test maximum runtime in TEST_MODE.
- [ ] Test restart delay.
- [ ] Test startup protection delay.
- [ ] Test float protection.
- [ ] Test Blynk bypass controls only under controlled conditions.

## Stage 5 — Integrated Test

- [ ] Test 2 HP motor with complete protection chain.
- [ ] Test 6 HP motor with complete protection chain.
- [ ] Test Auto Mode.
- [ ] Test Manual Mode.
- [ ] Test Blynk notification.
- [ ] Test loss/recovery of Wi-Fi after the controller has started.
- [ ] Verify contactor and electrical protection operation.

## Commissioning

Do not commission a live motor installation until the control panel, contactor,
overload protection, earthing, isolation and wiring have been independently checked.
