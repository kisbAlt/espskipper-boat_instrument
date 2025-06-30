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
bool forceFullRefresh = true;
bool gpsWasRefreshed = false;

void loop()
{
  unsigned long now = millis();

  if (forceFullRefresh || now - lastDisplayUpdate >= dispHandler.dispSettings.fullRefreshTime)
  {
    Serial.println("Redrawing UI");
    dispHandler.DrawUI(gpsHandler.lastNumOfSatellites, gpsHandler.stats, dispHandler.dispSettings);
    lastDisplayUpdate = now;
  }

  if (gpsHandler.GetGps()) {
    gpsWasRefreshed = true;
  }

  if (gpsWasRefreshed && (forceFullRefresh || now - lastSpeedUpdate >= dispHandler.dispSettings.speedRefreshTime))
  {
    if (dispHandler.dispSettings.useKnots)
    {
      dispHandler.DrawSpeed(gpsHandler.stats.lastSpeedKt);
    }
    else
    {
      dispHandler.DrawSpeed(gpsHandler.stats.lastSpeedKmph);
    }
    if (dispHandler.dispSettings.coursePartialUpdate) {
      Serial.println(gpsHandler.stats.lastCourse);
      dispHandler.PartialCourse(gpsHandler.stats.lastCourse);
    }
    lastSpeedUpdate = now;
    gpsWasRefreshed = false;
  }

  if (forceFullRefresh)
  {
    forceFullRefresh = false;
  }
  
  wifiHandler.HandleRequests(dispHandler.dispSettings, gpsHandler.stats, &forceFullRefresh);
}
