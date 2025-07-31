#include <Arduino.h>
#include <wifihandler.h>
#include <openEchoInterface.h>

DisplayHandler dispHandler;
GpsHandler gpsHandler;
WifiHandler wifiHandler;
OpenEchoInterface openEchoInterface;

// button PIN: 32
#define BUTTON_PIN 32 // GPIO connected to button
unsigned long lastDebounceTime = 0;
unsigned long pressStartTime = 0;
int clickCount = 0;
bool buttonIsPressed = false;
bool longPressDetected = false;

const unsigned long debounceDelay = 50;
const unsigned long clickTimeout = 500;
const unsigned long longPressDuration = 1000;
bool btnPressed = false;
bool btnReleased = false;
bool lastButtonState = HIGH;

void findFirstGps()
{

  while (gpsHandler.stats.lastLat == 0)
  {
    gpsHandler.GetGps();
    delay(500);
    Serial.println("cant find gps...");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  dispHandler.Init();
  gpsHandler.Init();
  delay(1000);
  // findFirstGps();

  wifiHandler.Init();

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // openEchoInterface.ReadPacket();
}

unsigned long lastSpeedUpdate = 0;
unsigned long lastDisplayUpdate = 0;
bool forceDispRefresh = true;
bool gpsWasRefreshed = false;

int handleClicks(unsigned long now)
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

  if ((now - lastDebounceTime) > debounceDelay)
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
    if (buttonIsPressed && !longPressDetected && (now - pressStartTime >= longPressDuration))
    {
      Serial.println("LONG PRESS");
      longPressDetected = true;
      clickCount = 0;
      return -1;
    }

    // Handle click count if no more clicks coming
    if (!buttonIsPressed && clickCount > 0 && (now - lastDebounceTime > clickTimeout))
    {
      int clicks = clickCount;
      clickCount = 0;
      return clicks;
    }
  }
  return 0;
}

void loop()
{
  unsigned long now = millis();

  int clicks = handleClicks(now);
  if (clicks != 0)
  {
    dispHandler.HandleButtonInput(clicks);
    forceDispRefresh = true;
  }

  if (gpsHandler.GetGps())
  {
    gpsWasRefreshed = true;
  }

  if (forceDispRefresh || now - lastSpeedUpdate >= dispHandler.dispSettings.speedRefreshTime)
  {
    dispHandler.DrawDisplay1(gpsHandler.stats, gpsHandler.lastNumOfSatellites, now);
    lastSpeedUpdate = now;
    gpsWasRefreshed = false;
  }

  if (forceDispRefresh || now - lastDisplayUpdate >= dispHandler.dispSettings.fullRefreshTime)
  {
    dispHandler.DrawDisplay2(gpsHandler.lastNumOfSatellites, gpsHandler.stats, dispHandler.dispSettings);
    lastDisplayUpdate = now;
  }

  if (forceDispRefresh)
  {
    forceDispRefresh = false;
  }

  wifiHandler.HandleRequests(dispHandler.dispSettings, gpsHandler.stats, &forceDispRefresh);
}
