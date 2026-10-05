# Wiring

## Current proposed wiring

```text
                         +3.3 V
                           |
                         R1 100 kOhm
                           |
                           +---------- GPIO4
                           |
                         C1 100 nF
                           |
                           +---------- GND

EFAK switch:

    COM ------------------------------- GND
    NC  ------------------------------- GPIO4
    NO  ------------------------------- NC (not connected)
```

## Power

```text
3 x AA battery holder

    + -------------------- ESP32-C3 5V / VBUS input
    - -------------------- ESP32-C3 GND
```

Recommended supply buffering:

```text
5V/VBUS -----------+---------------- ESP32-C3
                   |
                  +| C2 470 uF
                  -|
                   |
GND ---------------+---------------- ESP32-C3
```

Add a 100 nF ceramic capacitor close to the ESP32 supply pins.

## Contact behavior

Assuming COM-NC is the intended contact pair:

- Button released: COM-NC closed -> GPIO4 LOW
- Button pressed: COM-NC open -> R1 pulls GPIO4 HIGH

## Verification required

Before final assembly:

1. Identify the actual EFAK terminals.
2. Use a multimeter in continuity mode to identify COM, NC and NO.
3. Verify the exact ESP32-C3 board's 5V/VBUS input.
4. Verify GPIO4 is suitable for the intended deep-sleep wake-up configuration.
5. Confirm that the board's onboard regulator accepts the selected battery voltage.
