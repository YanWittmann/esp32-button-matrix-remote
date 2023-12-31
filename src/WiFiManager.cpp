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

void WiFiManager::reconnect() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    int i = 0;

    currentWiFi = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Waiting for WiFi: ");
        Serial.println(ssidArray[currentWiFi]);
        delay(500);
        if (i++ > 10) {
            i = 0;
            currentWiFi++;
            if (currentWiFi == numberOfNetworks) currentWiFi = 0;
            WiFi.begin(ssidArray[currentWiFi], passwordArray[currentWiFi]);
        }

        if (retryWiFi++ > 75) {
            espRestart("Failed to connect to WiFi - reboot and retry");
        }
    }

    Serial.print("WiFi connected: ");
    Serial.println(WiFi.localIP());
    retryWiFi = 0;
}

WiFiClient &WiFiManager::getWiFiClient() {
    return wifiClient;
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
