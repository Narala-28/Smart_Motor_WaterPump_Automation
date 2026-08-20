# Required Libraries

## Required

### ESP32 Arduino Core
Install the ESP32 board package through Arduino IDE Board Manager.

### Blynk
Required include:

```cpp
#include <BlynkSimpleEsp32.h>
```

Install the current Blynk library compatible with the selected ESP32 core.

### EmonLib
Required include:

```cpp
#include <EmonLib.h>
```

Used for RMS current calculation from SCT current sensors.

## Arduino IDE

Recommended workflow:

1. Install Arduino IDE.
2. Install ESP32 board support.
3. Install Blynk library.
4. Install EmonLib.
5. Select the correct ESP32 board.
6. Select the correct COM port.
7. Enter local Blynk/Wi-Fi credentials.
8. Compile before connecting a live motor.
