#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <vector>

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

std::vector<KeypadKeyEvent> unsentKeypadKeyEvents;

void broadcastKeypadEvent(KeypadKeyEvent event) {
    char mqttPayload[256];
    snprintf(mqttPayload, sizeof(mqttPayload), R"({"key":"%c","state":"%d","multiclick":"%d","device":"%s"})",
             event.key, event.state, event.multiclick, MQTT_DEVICE_NAME);

    setStatusLed(true);
    mqttManager.getClient().publish("heidelberg_home/remote_control/button_states", mqttPayload);
    setStatusLed(false);
}

void receiveKeypadEvent(KeypadKeyEvent event) {
    Serial.print("Keypad Event: ");
    Serial.print(event.key);
    Serial.print(" ");
    Serial.print(event.state);
    Serial.print(" ");
    Serial.println(event.multiclick);

    if (wifiManager.isConnected()) {
        broadcastKeypadEvent(event);
    } else {
        unsentKeypadKeyEvents.push_back(event);
        setStatusLed(false);
        delay(100);
        setStatusLed(true);
    }
}

void sendStoredEvents() {
    if (!wifiManager.isConnected()) return;
    if (unsentKeypadKeyEvents.empty()) return;

    Serial.print("Sending stored keypad events: ");
    Serial.println(unsentKeypadKeyEvents.size());

    for (const auto &event: unsentKeypadKeyEvents) {
        broadcastKeypadEvent(event);
    }

    unsentKeypadKeyEvents.clear();
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

KeypadManager keypadManager = KeypadManager(keys, rowPins, colPins, receiveKeypadEvent);

void initialConnectionEstablishingKeyCheck() {
    keypadManager.checkKeys();
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {
    }

    pinMode(STATUS_LED_PIN, OUTPUT);
    setStatusLed(true);

    Serial.println();
    Serial.println("Initializing ESP...");

    wifiManager.setStaticIp(static_device_ip);
    wifiManager.reconnect_intermediate_loop_callback(initialConnectionEstablishingKeyCheck);
    // mqttManager.reconnect();
    mqttManager.setCallback(callbackMqtt);
    mqttManager.loop(connectionLostCallback, connectionEstablishedCallback);

    delay(100);
    setStatusLed(false);

    sendStoredEvents();
}

// unsigned long lastLoopTime = 0;
// int loopCount = 0;

void loop() {
    wifiManager.loop(connectionLostCallback, connectionEstablishedCallback, broadcastDebugEvent);
    mqttManager.loop(connectionLostCallback, connectionEstablishedCallback);

    keypadManager.checkKeys();

    delay(SLEEP_MODE_AWAKE_LOOP_DELAY_MS);

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
