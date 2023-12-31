#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <PubSubClient.h>
#include "WiFiManager.h"

class MQTTManager {
public:
    MQTTManager(WiFiClient &wifiClient, const IPAddress &mqttServer, const char *deviceName, uint16_t port,
                const std::vector<const char *> &topicsToSubscribe);

    void reconnect();

    void setCallback(MQTT_CALLBACK_SIGNATURE);

    PubSubClient &getClient();

    void loop();

private:
    PubSubClient client;
    const char *deviceName;
    uint16_t port;
    int retryCount;
    std::vector<const char*> topicsToSubscribe;
};


#endif //MQTTMANAGER_H
