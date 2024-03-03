#include "WiFiManager.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

int currentWiFi;
int retryWiFi;

WiFiManager::WiFiManager(const char *deviceName, const char **ssidArray, const char **passwordArray,
                         int numberOfNetworks)
    : deviceName(deviceName), ssidArray(ssidArray), passwordArray(passwordArray), numberOfNetworks(numberOfNetworks),
      currentWiFi(0), retryCount(0) {
    WiFi.hostname(deviceName);
    WiFi.mode(WIFI_STA);
}

boolean WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

int WiFiManager::getSignalStrength() {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return -100;
}

unsigned long lastSignalStrengthCheckTime = millis();

void WiFiManager::loop(void (*connection_lost_callback)(), void (*connection_established_callback)(),
                       void (*broadcast_debug_event)(const char *key, const char *value)) {
    if (!isConnected()) {
        connection_lost_callback();
        reconnect();
        connection_established_callback();
    }

    if (const unsigned long now = millis(); now - lastSignalStrengthCheckTime > 10000) {
        lastSignalStrengthCheckTime = now;
        const int signalStrength = getSignalStrength();
        Serial.print("Signal strength: ");
        Serial.println(signalStrength);
        // broadcast_debug_event("signal_strength", String(signalStrength).c_str());
        if (signalStrength < -80) {
            Serial.println("Signal strength too low - reconnecting");
            disconnect();
            reconnect();
        }
    }
}

void WiFiManager::reconnect() {
    this->reconnect_intermediate_loop_callback([]() {
    });
}

void WiFiManager::reconnect_intermediate_loop_callback(std::function<void()> loop_callback) {
    if (isConnected()) return;

    const int loop_duration = 500;
    const int loop_callback_target_delay = 20;
    const int loop_callback_target_count = loop_duration / loop_callback_target_delay;
    Serial.print("Will be calling loop callback ");
    Serial.print(loop_callback_target_count);
    Serial.print(" times with a delay of ");
    Serial.print(loop_callback_target_delay);
    Serial.println("ms each.");

    int i = 0;
    currentWiFi = 0;

    do {
        if (i != 0) {
            for (int i = 0; i < loop_callback_target_count; ++i) {
                delay(loop_callback_target_delay);
                loop_callback();
            }

            if (retryWiFi++ > 75) {
                espRestart("Failed to connect to WiFi - reboot and retry");
            }
        }

        Serial.print("Waiting for WiFi: ");
        Serial.println(ssidArray[currentWiFi]);
        if (i++ > 10) {
            i = 0;
            currentWiFi++;
            if (currentWiFi == numberOfNetworks) currentWiFi = 0;
            WiFi.begin(ssidArray[currentWiFi], passwordArray[currentWiFi]);
        }
    } while (!isConnected());

    Serial.print("WiFi connected: ");
    Serial.println(WiFi.localIP());
    retryWiFi = 0;
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
    delay(1000);
}

WiFiClient &WiFiManager::getWiFiClient() {
    return wifiClient;
}

void WiFiManager::setStaticIp(IPAddress ip) {
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(192, 168, 1, 1);

    WiFi.config(ip, gateway, subnet, dns);
}

void WiFiManager::espRestart(const char *reason) {
    Serial.println("\n********\nRestart ESP now!!!\n********");
    Serial.print("Reason: ");
    Serial.println(reason);

    WiFi.mode(WIFI_OFF);
    EspClass::restart();

    // this loop is only for ensuring that the reset does not
    // have a side effect on going back to the mail loop()
    for (int s = 0; s < 60; s++) {
        delay(1000);
    }
}
