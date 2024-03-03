#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H


#include <ESP8266WiFi.h>

class WiFiManager {
public:
    WiFiManager(const char *deviceName, const char **ssidArray, const char **passwordArray, int numberOfNetworks);

    void loop(void(* connection_lost_callback)(), void(* connection_established_callback)(), void(* broadcast_debug_event)(const char *key, const char *value));

    void reconnect();

    void reconnect_intermediate_loop_callback(std::function<void()> loop_callback);

    static void disconnect();

    static auto getSignalStrength() -> int;

    static auto isConnected() -> boolean;

    static void espRestart(const char *reason);

    auto getWiFiClient() -> WiFiClient &;

    static void setStaticIp(IPAddress ip);

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
