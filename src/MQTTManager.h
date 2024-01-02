#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <PubSubClient.h>
#include "WiFiManager.h"

class MQTTManager {
public:
    MQTTManager(WiFiClient &wifiClient, const IPAddress &mqttServer, const char *deviceName, uint16_t port,
                const std::vector<const char *> &topicsToSubscribe);

    void loop(void(* connection_lost_callback)(), void(* connection_established_callback)());

    void reconnect();

    boolean isConnected();

    void setCallback(MQTT_CALLBACK_SIGNATURE);

    PubSubClient &getClient();

private:
    PubSubClient client;
    const char *deviceName;
    uint16_t port;
    int retryCount;
    std::vector<const char*> topicsToSubscribe;
};


#endif //MQTTMANAGER_H
