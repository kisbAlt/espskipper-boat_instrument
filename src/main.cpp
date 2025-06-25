#include <Arduino.h>
#include <wifihandler.h>

DisplayHandler dispHandler;
GpsHandler gpsHandler;
WifiHandler wifiHandler;

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
}

unsigned long lastSpeedUpdate = 0;
unsigned long lastDisplayUpdate = 0;
bool firstRun = true;
bool gpsWasRefreshed = false;

void loop()
{
  unsigned long now = millis();

  if (firstRun || now - lastDisplayUpdate >= dispHandler.dispSettings.fullRefreshTime)
  {
    Serial.println("Redrawing UI");
    dispHandler.DrawUI(gpsHandler.lastNumOfSatellites, gpsHandler.stats, dispHandler.dispSettings);
    lastDisplayUpdate = now;
  }

  if (gpsHandler.GetGps()) {
    gpsWasRefreshed = true;
  }

  if (gpsWasRefreshed && (firstRun || now - lastSpeedUpdate >= dispHandler.dispSettings.speedRefreshTime))
  {
    if (dispHandler.dispSettings.useKnots)
    {
      dispHandler.DrawSpeed(gpsHandler.stats.lastSpeedKt);
    }
    else
    {
      dispHandler.DrawSpeed(gpsHandler.stats.lastSpeedKmph);
    }
    lastSpeedUpdate = now;
    gpsWasRefreshed = false;
  }

  if (firstRun)
  {
    firstRun = false;
  }
  
  wifiHandler.HandleRequests(dispHandler.dispSettings, gpsHandler.stats);
}
