#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

#include <IPAddress.h>

// MQTT
#define MQTT_PORT 1883
#define MQTT_DEVICE_NAME "esp-yan-remote-client"
IPAddress MQTT_Server(192, 168, 1, 114);

// KEYPAD
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', '4'},
    {'5', '6', '7', '8'},
    {'9', 'A', 'B', 'C'},
    {'D', 'E', 'F', 'G'}
};

byte rowPins[KEYPAD_ROWS] = {16, 14, 12, 13}; // 4 3 2 1
byte colPins[KEYPAD_COLS] = {5, 4, 0, 2}; // 5 6 7 8

// WIFI
#define WIFI_NETWORK_COUNT 2
// const char *wifi_ssid[] = {"some-ssid-1", "some-ssid-2"};
// const char *wifi_password[] = {"some-password-1", "some-password-2"};
const char *wifi_ssid[] = {"Lauchzwiebeln", "Lauchzwiebeln"};
const char *wifi_password[] = {"4321NYUHW@hd", "4321NYUHW@hd"};

#endif //PROJECTCONFIG_H
