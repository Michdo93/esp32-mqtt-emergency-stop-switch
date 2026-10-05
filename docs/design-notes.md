# Design Notes

## Design philosophy

The device is intentionally designed as a wireless human-interface button rather than as a safety device.

The large mushroom head makes the button easy to find and operate, while MQTT provides integration with home automation or other network services.

## Why use the NC contact?

Using COM-NC gives the firmware a useful default state:

- released -> electrical connection to ground -> LOW
- pressed -> connection opens -> pull-up -> HIGH

The normally closed contact also makes an open wire detectable as a logical active/pressed condition. This is useful as an input diagnostic, although it must not be interpreted as a safety function.

## Why an external pull-up?

An external 100 kOhm pull-up makes the intended input circuit explicit and keeps the switch wiring simple. The 100 nF capacitor provides additional electrical filtering.

The exact resistor/capacitor values can be changed after testing the actual switch and battery lifetime.

## Power budget considerations

Deep sleep is intended to dominate the operating time. Wi-Fi transmission is the main energy cost. The 470 uF capacitor is intended to help with short supply-current peaks.

Actual battery lifetime must be measured on the final hardware because it depends strongly on:

- Wi-Fi signal strength
- access point behavior
- MQTT connection time
- ESP32 board regulator quiescent current
- battery chemistry
- temperature
- button activity

## Future improvements

- final schematic
- exact board-specific GPIO wake-up implementation
- battery ADC measurement
- MQTT availability/status topic
- MQTT last-will handling where appropriate
- configurable retry behavior
- secure MQTT/TLS option
- local configuration file for credentials
- enclosure and PCB/stripboard layout
