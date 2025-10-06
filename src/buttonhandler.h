#pragma once
#include <Arduino.h>
// button PIN: 32
#define BUTTON_PIN 32 // GPIO connected to button
#define DEBOUNCE_DELAY 50
#define CLICKTIMEOUT 500
#define LONGPRESS_DURATION 1000

class ButtonHandler
{
private:
    unsigned long lastDebounceTime = 0;
    unsigned long pressStartTime = 0;
    int clickCount = 0;
    bool buttonIsPressed = false;
    bool longPressDetected = false;
    bool btnPressed = false;
    bool btnReleased = false;
    bool lastButtonState = HIGH;

public:
    ButtonHandler();
    int HandleClicks(unsigned long &now);
};