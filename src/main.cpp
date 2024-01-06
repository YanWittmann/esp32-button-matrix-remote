#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "ProjectConfig.h"
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "KeypadManager.h"


WiFiManager wifiManager(MQTT_DEVICE_NAME, wifi_ssid, wifi_password, WIFI_NETWORK_COUNT);
MQTTManager mqttManager(wifiManager.getWiFiClient(), MQTT_Server, MQTT_DEVICE_NAME, MQTT_PORT, {});
// MQTTManager mqttManager(wifiManager.getWiFiClient(), MQTT_Server, MQTT_DEVICE_NAME, MQTT_PORT, {"test/nodered"});

void callbackMqtt(const char *topic, const byte *payload, const unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print(static_cast<char>(payload[i]));
    }
    Serial.println();
}

void setStatusLed(const bool isOn) {
    digitalWrite(STATUS_LED_PIN, isOn ? HIGH : LOW);
}

void connectionLostCallback() {
    setStatusLed(true);
}

void connectionEstablishedCallback() {
    setStatusLed(false);
}

unsigned long lastWakeupTime = millis();

bool isWokenUp() {
    return millis() - lastWakeupTime < SLEEP_MODE_TIMEOUT_MS;
}

void wakeUp() {
    if (!isWokenUp()) {
        setStatusLed(true);
        Serial.println("Waking up");
    }

    lastWakeupTime = millis();
}

void broadcastKeypadEvent(const char key, const int state, const int multiclick) {
    char mqttPayload[256];
    snprintf(mqttPayload, sizeof(mqttPayload), R"({"key":"%c","state":"%d","multiclick":"%d","device":"%s"})",
             key, state, multiclick, MQTT_DEVICE_NAME);

    Serial.print("Keypad Event: ");
    Serial.println(mqttPayload);

    setStatusLed(true);
    mqttManager.getClient().publish("heidelberg_home/remote_control/button_states", mqttPayload);
    setStatusLed(false);

    wakeUp();
}

void broadcastDebugEvent(const char *key, const char *value) {
    char mqttPayload[256];
    snprintf(mqttPayload, sizeof(mqttPayload), R"({"key":"%s","value":"%s","device":"%s"})",
             key, value, MQTT_DEVICE_NAME);

    Serial.print("Debug Event: ");
    Serial.println(mqttPayload);

    setStatusLed(true);
    mqttManager.getClient().publish("heidelberg_home/remote_control/debug", mqttPayload);
    setStatusLed(false);
}

KeypadManager keypadManager = KeypadManager(keys, rowPins, colPins, broadcastKeypadEvent);

void setup() {
    Serial.begin(9600);
    while (!Serial) {
    }

    pinMode(STATUS_LED_PIN, OUTPUT);
    setStatusLed(true);

    Serial.println();
    Serial.println("Initializing ESP...");

    wifiManager.reconnect();
    mqttManager.reconnect();
    mqttManager.setCallback(callbackMqtt);

    delay(1500);
    setStatusLed(false);
}

// unsigned long lastLoopTime = 0;
// int loopCount = 0;

void loop() {
    wifiManager.loop(connectionLostCallback, connectionEstablishedCallback, broadcastDebugEvent);
    mqttManager.loop(connectionLostCallback, connectionEstablishedCallback);

    keypadManager.checkKeys();

    if (isWokenUp()) {
        delay(SLEEP_MODE_AWAKE_LOOP_DELAY_MS);
    } else {
        delay(SLEEP_MODE_ASLEEP_LOOP_DELAY_MS);
    }

    // serial print loop time
    // loopCount++;
    // const unsigned long now = millis();
    // if (now - lastLoopTime > 1000) {
    //     Serial.print("Loops in last 1 second: ");
    //     Serial.print(loopCount);
    //     Serial.print(", ms per loop: ");
    //     Serial.println((float) (now - lastLoopTime) / (float) loopCount); // time in ms
    //     lastLoopTime = now;
    //     loopCount = 0;
    // }
}
