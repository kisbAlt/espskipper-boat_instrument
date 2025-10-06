#include <buttonhandler.h>

ButtonHandler::ButtonHandler()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

int ButtonHandler::HandleClicks(unsigned long &now)
{

    bool currentState = digitalRead(BUTTON_PIN);

    // Debounce check
    if (currentState != lastButtonState)
    {
        lastDebounceTime = now;

        if (currentState == LOW)
        {
            btnPressed = true;
            btnReleased = false;
        }
        else
        {
            btnReleased = true;
            btnPressed = false;
        }
        lastButtonState = currentState;
    }

    if ((now - lastDebounceTime) > DEBOUNCE_DELAY)
    {
        if (btnPressed)
        {
            // Button just pressed
            pressStartTime = now;
            buttonIsPressed = true;
            longPressDetected = false;
            btnPressed = false;
        }

        if (btnReleased && buttonIsPressed)
        {
            // Button just released
            btnReleased = false;
            buttonIsPressed = false;
            if (!longPressDetected)
            {
                clickCount++;
                lastDebounceTime = now; // restart click timeout
            }
        }

        // Long press detection
        if (buttonIsPressed && !longPressDetected && (now - pressStartTime >= LONGPRESS_DURATION))
        {
            Serial.println("LONG PRESS");
            longPressDetected = true;
            clickCount = 0;
            return -1;
        }

        // Handle click count if no more clicks coming
        if (!buttonIsPressed && clickCount > 0 && (now - lastDebounceTime > CLICKTIMEOUT))
        {
            int clicks = clickCount;
            clickCount = 0;
            return clicks;
        }
    }
    return 0;
}