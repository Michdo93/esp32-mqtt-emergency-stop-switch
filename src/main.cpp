#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_sleep.h"

// -----------------------------------------------------------------------------
// User configuration
// -----------------------------------------------------------------------------

const char* WIFI_SSID     = "CHANGE_ME";
const char* WIFI_PASSWORD = "CHANGE_ME";

const char* MQTT_SERVER   = "192.168.0.5";
const uint16_t MQTT_PORT  = 1883;
const char* MQTT_USER     = "";
const char* MQTT_PASSWORD = "";
const char* MQTT_TOPIC    = "buttons/efak/state";

// Intended input pin. Verify this against the exact ESP32-C3 board used.
constexpr gpio_num_t BUTTON_PIN = GPIO_NUM_4;

// GPIO4 is pulled HIGH by the external 100 kOhm resistor and is pulled LOW
// through the switch when COM-NC is closed.
constexpr uint32_t DEBOUNCE_MS = 50;
constexpr uint32_t WIFI_TIMEOUT_MS = 10000;
constexpr uint32_t MQTT_TIMEOUT_MS = 5000;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

bool readStableButtonState()
{
    const bool first = digitalRead(BUTTON_PIN) == HIGH;
    delay(DEBOUNCE_MS);
    const bool second = digitalRead(BUTTON_PIN) == HIGH;

    // If the two samples disagree, use the second sample. The next wake cycle
    // will provide another opportunity to report the stable state.
    return second;
}

bool connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_TIMEOUT_MS) {
        delay(100);
    }

    return WiFi.status() == WL_CONNECTED;
}

bool connectMQTT()
{
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

    const uint32_t start = millis();
    while (!mqttClient.connected() && millis() - start < MQTT_TIMEOUT_MS) {
        const String clientId = "efak-esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

        bool connected;
        if (MQTT_USER[0] != '\0') {
            connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
        } else {
            connected = mqttClient.connect(clientId.c_str());
        }

        if (!connected) {
            delay(250);
        }
    }

    return mqttClient.connected();
}

void publishState(bool pressed)
{
    const char* payload = pressed ? "pressed" : "released";

    if (connectMQTT()) {
        mqttClient.publish(MQTT_TOPIC, payload, true);
        mqttClient.loop();
        delay(50);
        mqttClient.disconnect();
    }
}

void prepareForSleep(bool currentPressed)
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    // Wake only when the input changes to the opposite electrical level.
    // This prevents an immediate wake-up loop when the button remains in the
    // same state after the MQTT publication.
    const esp_gpio_wakeup_gpio_level_t wakeLevel = currentPressed
        ? ESP_GPIO_WAKEUP_GPIO_LOW
        : ESP_GPIO_WAKEUP_GPIO_HIGH;

    esp_deep_sleep_enable_gpio_wakeup(
        1ULL << BUTTON_PIN,
        wakeLevel
    );

    delay(20);
    esp_deep_sleep_start();
}

// -----------------------------------------------------------------------------
// Setup / loop
// -----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    delay(100);

    pinMode(BUTTON_PIN, INPUT);

    const bool pressed = readStableButtonState();

    Serial.printf("Button state: %s\n", pressed ? "pressed" : "released");

    if (connectWiFi()) {
        publishState(pressed);
    } else {
        Serial.println("Wi-Fi connection failed; entering deep sleep.");
    }

    prepareForSleep(pressed);
}

void loop()
{
    // The device should never normally reach loop(), because setup() enters
    // deep sleep after publishing the current state.
    delay(1000);
}
