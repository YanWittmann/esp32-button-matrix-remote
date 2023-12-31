#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Keypad.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <WiFiClient.h>


#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
#define KEYPAD_MULTI_CLICK_MAX_INTERVAL 350

#define MQTT_PORT 1883
#define MQTT_DEVICE_NAME "esp-yan-remote-client"

#define WIFIs 2

// SYSTEM
void espRestart(const char *reason) {
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

// WIFI
// WiFi array of SSID's and passwords
const char *wifi_ssid[] = {"Lauchzwiebeln", "Lauchzwiebeln"};
const char *wifi_password[] = {"4321NYUHW@hd", "4321NYUHW@hd"};
int currentWiFi = 0;

WiFiClient wifiClient;

int retryWiFi = 0;

void reconnectWiFi() {
    int i = 0;

    currentWiFi = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Waiting for WLAN: ");
        Serial.println(wifi_ssid[currentWiFi]);
        delay(500);
        if (i++ > 10) {
            i = 0;
            currentWiFi++;
            if (currentWiFi == WIFIs) currentWiFi = 0;
            WiFi.begin(wifi_ssid[currentWiFi], wifi_password[currentWiFi]);
        }

        if (retryWiFi++ > 75) {
            espRestart("Failed to connect to WiFi - reboot and retry");
        }
    }

    Serial.print("WLAN connected: ");
    Serial.println(WiFi.localIP());
    retryWiFi = 0;
}


// MQTT
PubSubClient client(wifiClient);
EthernetClient ethClient;
IPAddress MQTT_Server(192, 168, 1, 114);

int retryMQTT = 0;

void reconnectMqtt() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection... ");

        if (client.connect(MQTT_DEVICE_NAME)) {
            Serial.println("MQTT connected");
            retryMQTT = 0;

            client.subscribe("ESP-Time/Time");
        } else {
            Serial.print("MQTT failed to connect, rc=");
            Serial.print(client.state());
            Serial.println(" ...retry in 1 second");
            delay(1000);

            if (retryMQTT++ > 75) {
                espRestart("Failed to connect to MQTT - reboot and retry");
            }
        }
    }
}

void callbackMqtt(const char *topic, const byte *payload, const unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print(static_cast<char>(payload[i]));
    }
    Serial.println();
}

// KEYPAD
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', '4'},
    {'5', '6', '7', '8'},
    {'9', 'A', 'B', 'C'},
    {'D', 'E', 'F', 'G'}
};

byte rowPins[KEYPAD_ROWS] = {16, 14, 12, 13}; // 4 3 2 1
byte colPins[KEYPAD_COLS] = {5, 4, 0, 2}; // 5 6 7 8

unsigned long lastKeyPressTime = 0;
unsigned long lastKeyReleaseTime = 0;
int keyPressCount = 0;
char lastKey = '\0';
bool keypadActionIsNotHoldingDown = false;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

int keypadCheckMultiClick(const char key) {
    const unsigned long currentTime = millis();
    if (key == lastKey && currentTime - lastKeyPressTime <= KEYPAD_MULTI_CLICK_MAX_INTERVAL) {
        keyPressCount++;
    } else {
        keyPressCount = 1;
    }
    lastKeyPressTime = currentTime;
    lastKey = key;

    if (keyPressCount > 3) {
        keyPressCount = 3;
    }
    return keyPressCount;
}

void broadcastKeypadEvent(const char key, const int state, const int multiclick) {
    char mqttPayload[256];
    snprintf(mqttPayload, sizeof(mqttPayload), R"({"key":"%c","state":"%d","multiclick":"%d","device":"%s"})",
             key, state, multiclick, MQTT_DEVICE_NAME);

    Serial.print("Keypad Event: ");
    Serial.println(mqttPayload);

    client.publish("heidelberg_home/remote_control/button_states", mqttPayload);
}

// arduino

void setup() {
    Serial.begin(9600);
    while (!Serial) {
    }

    Serial.println();
    Serial.println("Initializing ESP...");

    WiFi.hostname(MQTT_DEVICE_NAME);
    WiFi.mode(WIFI_STA);

    if (WiFi.status() != WL_CONNECTED)
        reconnectWiFi();

    client.setServer(MQTT_Server, MQTT_PORT);
    client.setCallback(callbackMqtt);

    delay(1500);
}

void loop() {
    if (!client.connected()) {
        reconnectMqtt();
    }
    client.loop();

    if (WiFi.status() != WL_CONNECTED) {
        reconnectWiFi();
    }

    if (keypad.getKeys()) {
        for (const auto &i: keypad.key) {
            if (i.stateChanged) {
                switch (i.kstate) {
                    case PRESSED:
                        keypadCheckMultiClick(i.kchar);
                        lastKeyReleaseTime = 0;
                        keypadActionIsNotHoldingDown = true;
                        break;
                    case RELEASED:
                        broadcastKeypadEvent(lastKey, RELEASED, keyPressCount);
                        lastKeyReleaseTime = millis();
                        break;
                    case HOLD:
                        broadcastKeypadEvent(lastKey, HOLD, keyPressCount);
                        lastKeyReleaseTime = 0;
                        keypadActionIsNotHoldingDown = false;
                    default:
                        break;
                }
            }
        }
    }

    if (lastKeyReleaseTime > 0 && keypadActionIsNotHoldingDown && (millis() - lastKeyReleaseTime) >= KEYPAD_MULTI_CLICK_MAX_INTERVAL) {
        broadcastKeypadEvent(lastKey, PRESSED, keyPressCount);
        lastKeyReleaseTime = 0;
    }
}
