#ifndef KEYPADMANAGER_H
#define KEYPADMANAGER_H

#include <Keypad.h>

struct KeypadKeyEvent {
    char key;
    int state;
    int multiclick;
};

typedef void (*KeypadEventCallback)(KeypadKeyEvent event);

template<size_t numRows, size_t numCols>
class KeypadManager {
public:
    KeypadManager(const char (&keys)[numRows][numCols], byte *rowPins, byte *colPins, KeypadEventCallback callback);

    void checkKeys();

private:
    Keypad keypad;
    KeypadEventCallback eventCallback;
    unsigned long lastKeyPressTime;
    unsigned long lastKeyReleaseTime;
    int keyPressCount;
    char lastKey;
    bool isNotHoldingDown;

    unsigned long multiClickMaxInterval;

    int checkMultiClick(char key);
};

#define KEYPAD_MULTI_CLICK_MAX_INTERVAL 350

template<size_t numRows, size_t numCols>
KeypadManager<numRows, numCols>::KeypadManager(const char (&keys)[numRows][numCols], byte *rowPins, byte *colPins,
                                               const KeypadEventCallback callback)
    : keypad(Keypad(makeKeymap(keys), rowPins, colPins, numRows, numCols)),
      eventCallback(callback), lastKeyPressTime(0), lastKeyReleaseTime(0), keyPressCount(0), lastKey('\0'),
      isNotHoldingDown(false), multiClickMaxInterval(KEYPAD_MULTI_CLICK_MAX_INTERVAL) {
}

template<size_t numRows, size_t numCols>
void KeypadManager<numRows, numCols>::checkKeys() {
    if (keypad.getKeys()) {
        for (const auto &keyEvent: keypad.key) {
            if (keyEvent.stateChanged) {
                switch (keyEvent.kstate) {
                    case PRESSED:
                        checkMultiClick(keyEvent.kchar);
                        lastKeyReleaseTime = 0;
                        isNotHoldingDown = true;
                        break;
                    case RELEASED:
                        if (eventCallback) eventCallback({lastKey, RELEASED, keyPressCount});
                        lastKeyReleaseTime = millis();
                        break;
                    case HOLD:
                        if (eventCallback) eventCallback({lastKey, HOLD, keyPressCount});
                        lastKeyReleaseTime = 0;
                        isNotHoldingDown = false;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    if (lastKeyReleaseTime > 0 && isNotHoldingDown && (millis() - lastKeyReleaseTime) >= multiClickMaxInterval) {
        if (eventCallback) eventCallback({lastKey, PRESSED, keyPressCount});
        lastKeyReleaseTime = 0;
    }
}

template<size_t numRows, size_t numCols>
int KeypadManager<numRows, numCols>::checkMultiClick(char key) {
    const unsigned long currentTime = millis();
    if (key == lastKey && currentTime - lastKeyPressTime <= multiClickMaxInterval) {
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

#endif //KEYPADMANAGER_H
