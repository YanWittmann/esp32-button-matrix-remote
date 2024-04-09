#include <Arduino.h>
#include "ProjectConfig.h"

#ifndef BUTTON_MATRIX_REMOTE_ESP32_LEDPATTERN_H
#define BUTTON_MATRIX_REMOTE_ESP32_LEDPATTERN_H

static bool currentLedState = false;

class LedPattern {
public:
    unsigned long onDuration;
    unsigned long offDuration;
    unsigned long lastChangeTime = 0;
    unsigned long pauseEndTime = 0;

    LedPattern(unsigned long onDuration, unsigned long offDuration)
            : onDuration(onDuration), offDuration(offDuration) {}

    void update() {
        unsigned long currentTime = millis();
        if (currentTime < pauseEndTime) return;

        if ((currentLedState && (currentTime - lastChangeTime >= onDuration)) ||
            (!currentLedState && (currentTime - lastChangeTime >= offDuration))) {
            currentLedState = !currentLedState;
            setStatusLed(currentLedState);
            lastChangeTime = currentTime;
        }
    }

    void pause(unsigned long duration) {
        pauseEndTime = millis() + duration;
    }

private:
    void setStatusLed(bool isOn) {
        digitalWrite(STATUS_LED_PIN, isOn ? HIGH : LOW);
    }
};


#endif //BUTTON_MATRIX_REMOTE_ESP32_LEDPATTERN_H
