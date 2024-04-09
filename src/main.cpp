#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <vector>

#include "ProjectConfig.h"
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "KeypadManager.h"
#include "LedPattern.h"


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

unsigned long lastKeyInputTime = 0;

static LedPattern ledOnPattern(UINT32_MAX, 0);
static LedPattern ledOffPattern(0, UINT32_MAX);
static LedPattern ledFastBlinkPattern(100, 100);
static LedPattern ledSlowBlinkPattern(500, 500);
static LedPattern ledIrregular1BlinkPattern(100, 500);
static LedPattern ledIrregular2BlinkPattern(500, 100);

LedPattern* activePattern = &ledOnPattern;

void setActiveLedPattern(LedPattern* pattern) {
    if (pattern != nullptr) {
        activePattern = pattern;
        activePattern->update();
        Serial.print("Set active LED pattern ");
        Serial.print(pattern->onDuration);
        Serial.print(" ");
        Serial.println(pattern->offDuration);
    }
}

void wifiConnectionLostCallback() {
    setActiveLedPattern(&ledIrregular2BlinkPattern);
}

void wifiConnectionEstablishedCallback() {
    setActiveLedPattern(&ledOffPattern);
}

void mqttConnectionLostCallback() {
    setActiveLedPattern(&ledIrregular1BlinkPattern);
}

void mqttConnectionEstablishedCallback() {
    setActiveLedPattern(&ledOffPattern);
}

std::vector<KeypadKeyEvent> unsentKeypadKeyEvents;

void broadcastKeypadEvent(KeypadKeyEvent event) {
    char mqttPayload[256];
    snprintf(mqttPayload, sizeof(mqttPayload), R"({"key":"%c","state":"%d","multiclick":"%d","device":"%s"})",
             event.key, event.state, event.multiclick, MQTT_DEVICE_NAME);

    lastKeyInputTime = millis();
    setActiveLedPattern(&ledOnPattern);
    mqttManager.getClient().publish("heidelberg_home/remote_control/button_states", mqttPayload);
    setActiveLedPattern(&ledOffPattern);
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
        setActiveLedPattern(&ledOffPattern);
        delay(100);
        setActiveLedPattern(&ledOnPattern);
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

    setActiveLedPattern(&ledOnPattern);
    mqttManager.getClient().publish("heidelberg_home/remote_control/debug", mqttPayload);
    setActiveLedPattern(&ledOffPattern);
}

KeypadManager keypadManager = KeypadManager(keys, rowPins, colPins, receiveKeypadEvent);

void initialConnectionEstablishingKeyCheck() {
    activePattern->update();
    keypadManager.checkKeys();
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {
    }

    pinMode(STATUS_LED_PIN, OUTPUT);
    setActiveLedPattern(&ledOnPattern);
    lastKeyInputTime = millis();

    Serial.println();
    Serial.println("Initializing ESP...");

    wifiManager.setStaticIp(static_device_ip);
    wifiConnectionLostCallback(); // make LED blink
    wifiManager.reconnect_intermediate_loop_callback(initialConnectionEstablishingKeyCheck);
    // mqttManager.reconnect();
    mqttManager.setCallback(callbackMqtt);
    mqttManager.loop(mqttConnectionLostCallback, mqttConnectionEstablishedCallback);

    delay(100);
    setActiveLedPattern(&ledOffPattern);
    lastKeyInputTime = millis();

    sendStoredEvents();
}

// unsigned long lastLoopTime = 0;
// int loopCount = 0;

void loop() {
    wifiManager.loop(wifiConnectionLostCallback, wifiConnectionEstablishedCallback, broadcastDebugEvent);
    mqttManager.loop(mqttConnectionLostCallback, mqttConnectionEstablishedCallback);

    keypadManager.checkKeys();

    delay(SLEEP_MODE_AWAKE_LOOP_DELAY_MS);

    if (activePattern != &ledFastBlinkPattern && millis() - lastKeyInputTime > STATUS_LED_INACTIVITY_STATE) {
        setActiveLedPattern(&ledFastBlinkPattern);
    } else {
        activePattern->update();
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
