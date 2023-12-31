#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H


#include <ESP8266WiFi.h>

class WiFiManager {
public:
    WiFiManager(const char *deviceName, const char **ssidArray, const char **passwordArray, int numberOfNetworks);

    void reconnect();

    static void espRestart(const char *reason);

    WiFiClient &getWiFiClient();

private:
    const char *deviceName;
    const char **ssidArray;
    const char **passwordArray;
    int numberOfNetworks;
    int currentWiFi;
    int retryCount;
    WiFiClient wifiClient;
};

#endif //WIFIMANAGER_H
