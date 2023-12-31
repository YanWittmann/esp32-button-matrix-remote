#include "MQTTManager.h"

int retryMQTT = 0;

MQTTManager::MQTTManager(WiFiClient &wifiClient, const IPAddress &mqttServer, const char *deviceName, uint16_t port,
                         const std::vector<const char *> &topicsToSubscribe)
    : client(wifiClient), deviceName(deviceName), port(port), retryCount(0), topicsToSubscribe(topicsToSubscribe) {
    client.setServer(mqttServer, port);
}


void MQTTManager::reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");

        if (client.connect(deviceName)) {
            Serial.println("MQTT connected");
            retryCount = 0;

            for (const char* &topic: topicsToSubscribe) {
                client.subscribe(topic);
            }
        } else {
            Serial.print("MQTT failed to connect, rc=");
            Serial.print(client.state());
            Serial.println(" ...retry in 1 second");
            delay(1000);

            if (retryCount++ > 75) {
                WiFiManager::espRestart("Failed to connect to MQTT - reboot and retry");
            }
        }
    }
}

void MQTTManager::loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}

void MQTTManager::setCallback(MQTT_CALLBACK_SIGNATURE) {
    client.setCallback(std::move(callback));
}

PubSubClient &MQTTManager::getClient() {
    return client;
}
