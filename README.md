# ESP32 MQTT Emergency Stop Switch

A wireless MQTT button based on an ESP32-C3 and an IP65 mushroom-head EFAK foot/hand switch.

> **Important:** This project uses the mushroom switch as a **wireless input device**, not as a safety-rated emergency-stop circuit. The ESP32, Wi-Fi, MQTT, batteries and software must **never** be relied upon for a safety function where failure could cause injury, death, or property damage. For real emergency-stop applications, use a properly designed and certified safety circuit.

## Project goal

The physical device deliberately looks like an emergency-stop switch, but its electrical purpose is simply to provide a robust, highly visible wireless button.

When the mushroom button is pressed, the ESP32 connects to Wi-Fi, publishes an MQTT state, and returns to deep sleep. When the button is released, it publishes the corresponding released state.

The design is intended to be battery powered and to minimize energy consumption by keeping the ESP32 in deep sleep whenever possible.

## Features

- ESP32-C3 based
- Wireless operation via Wi-Fi
- MQTT state reporting
- Configurable Wi-Fi SSID and password
- Configurable MQTT broker, port, credentials and topic
- Mushroom-head EFAK IP65 switch
- Uses the switch's potential-free contact
- Battery powered
- Deep-sleep operation for low idle power consumption
- Wake-up on button state change
- MQTT `pressed` / `released` states
- MQTT retained state
- Optional battery-voltage reporting planned
- No LED required
- Simple hardware with only a few passive components

## Hardware

### Main components

| Component | Quantity | Suggested specification |
|---|---:|---|
| ESP32-C3 development board | 1 | ESP32-C3 SuperMini or equivalent |
| EFAK mushroom-head switch | 1 | IP65 foot/hand switch, purchased separately |
| Battery holder | 1 | 3 × AA cells, preferably with an integrated switch |
| AA batteries | 3 | Alkaline cells for the initial build |
| R1 | 1 | 100 kOhm |
| C1 | 1 | 100 nF |
| C2 | 1 | 470 uF, >= 6.3 V |
| Wire | as required | suitable for the enclosure and switch |

A small additional 100 nF decoupling capacitor close to the ESP32 supply pins is also recommended.

### Why three AA batteries?

Three AA alkaline cells provide approximately 4.5 V nominally. This is a practical supply for ESP32 development boards that expose a suitable 5 V/VBUS input and provide their own 3.3 V regulator.

A 9 V block battery is deliberately not recommended for this project. A CR2032 coin cell is also unsuitable for the Wi-Fi transmission current required by an ESP32.

The exact power connection depends on the ESP32-C3 board used. **Do not connect 4.5 V directly to the ESP32's 3.3 V pin.**

## Switch wiring

The intended input circuit uses the switch's normally closed (NC) contact:

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
    NO  ------------------------------- not connected
```

With this arrangement:

| Mushroom button | Switch contact | GPIO4 | MQTT state |
|---|---|---:|---|
| Released | COM-NC closed | LOW | `released` |
| Pressed | COM-NC open | HIGH | `pressed` |

### Important: verify the actual EFAK terminal assignment

The exact EFAK product listing does not provide sufficiently reliable terminal documentation. Before making the final wiring, inspect the switch for markings such as `COM`, `NC`, and `NO`, or verify the contacts with a multimeter.

**Do not assume terminal numbers from another model are identical to the purchased switch.**

The repository intentionally treats the contact assignment as a hardware verification step until a photo or datasheet for the exact switch is available.

## Power and decoupling

The proposed supply arrangement is:

```text
3 x AA battery holder

    + -------------------- ESP32-C3 5V / VBUS input
    - -------------------- ESP32-C3 GND
```

Add a 470 uF electrolytic capacitor across the ESP32 supply input and GND. The capacitor helps buffer short current peaks when Wi-Fi starts transmitting.

A 100 nF ceramic capacitor close to the ESP32 supply is also recommended.

Example:

```text
5V/VBUS -----------+---------------- ESP32-C3
                   |
                  +| C2 470 uF
                  -|
                   |
GND ---------------+---------------- ESP32-C3

3.3V -------- C3 100 nF -------- GND
```

The exact location of these components should be kept electrically close to the ESP32 board.

## Operating principle

The device is intended to spend most of its time asleep:

```text
                 +------------------+
                 |   Deep Sleep     |
                 +--------+---------+
                          |
                     GPIO wake-up
                          |
                          v
                 +------------------+
                 | Read button state |
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | Start Wi-Fi       |
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | Connect to MQTT   |
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | Publish state     |
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | Disconnect / sleep|
                 +------------------+
```

This approach avoids keeping the Wi-Fi radio continuously active.

## MQTT

The firmware uses a configurable MQTT topic. The default example is:

```text
buttons/efak/state
```

The payloads are:

```text
pressed
```

and:

```text
released
```

The state is published as a retained MQTT message so that an MQTT client can obtain the latest known state immediately after subscribing.

### Example

```text
Topic:   buttons/efak/state
Payload: pressed
Retain:  true
```

On release:

```text
Topic:   buttons/efak/state
Payload: released
Retain:  true
```

## Configuration

The firmware is intentionally configured through constants near the top of the source file.

Example:

```cpp
const char* WIFI_SSID     = "MyWiFi";
const char* WIFI_PASSWORD = "MyWiFiPassword";

const char* MQTT_SERVER   = "192.168.0.5";
const uint16_t MQTT_PORT  = 1883;

const char* MQTT_USER     = "";
const char* MQTT_PASSWORD = "";

const char* MQTT_TOPIC    = "buttons/efak/state";
```

For production use, credentials should preferably be kept outside the committed source code, for example in a local configuration file excluded through `.gitignore`.

## Suggested MQTT topics

The initial firmware uses one state topic:

```text
buttons/efak/state
```

A future version can additionally publish:

```text
buttons/efak/battery
buttons/efak/status
```

For example:

```text
buttons/efak/state    -> pressed
buttons/efak/battery  -> 4.21
buttons/efak/status   -> online
```

## Battery monitoring

Battery-voltage monitoring is intentionally left as a planned feature until the exact ESP32-C3 board and its available ADC pins are fixed.

A resistor divider will be required if the battery voltage can exceed the ESP32 ADC input range. The divider values should be selected for both the ADC range and low quiescent current.

## Debouncing

Mechanical switches bounce. The firmware should therefore not immediately treat the first electrical transition as a stable state.

The implementation provides a software debounce period before publishing the final state.

## ESP32 deep sleep and wake-up

The ESP32-C3 supports low-power sleep modes and GPIO wake-up. The exact GPIO used for wake-up must be supported by the selected ESP32-C3 board and firmware configuration.

The current project uses GPIO4 as the intended button input. Verify the pin mapping against the exact ESP32-C3 board before assembly.

## Safety disclaimer

This project is **not a safety-rated emergency-stop system**.

The mushroom-head switch is used only as a user interface. The wireless link can fail because of:

- dead batteries
- Wi-Fi outages
- MQTT broker failures
- software crashes
- radio interference
- ESP32 hardware failure
- network configuration errors
- power-supply problems

A real emergency-stop function must use an appropriate hardwired or certified safety architecture and must not depend on this project.

## Repository structure

```text
esp32-mqtt-emergency-stop-switch/
├── README.md
├── LICENSE
├── .gitignore
├── platformio.ini
├── src/
│   └── main.cpp
├── hardware/
│   └── wiring.md
└── docs/
    └── design-notes.md
```

## Building the firmware

The project is prepared for PlatformIO.

Install PlatformIO, connect an ESP32-C3 development board, verify the board definition in `platformio.ini`, and build/upload the firmware.

Example commands:

```bash
pio run
pio run --target upload
pio device monitor
```

## Development status

- [x] Project concept
- [x] MQTT state model
- [x] Battery-powered architecture
- [x] Deep-sleep concept
- [x] Initial wiring concept
- [x] English README
- [x] Initial firmware
- [ ] Verify exact EFAK terminal assignment
- [ ] Verify exact ESP32-C3 board/pin mapping
- [ ] Finalize GPIO wake-up configuration
- [ ] Finalize battery-voltage measurement
- [ ] Produce final schematic
- [ ] Build and test prototype
- [ ] Measure real battery lifetime

## License

See `LICENSE`.
