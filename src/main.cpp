#include <Arduino.h>
#include <wifihandler.h>
#include <openEchoInterface.h>
#include <buttonhandler.h>

DisplayHandler dispHandler;
GpsHandler gpsHandler;
WifiHandler wifiHandler;
OpenEchoInterface openEchoInterface;
ButtonHandler buttonHandler;

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
  openEchoInterface.Init();
  Serial.println("Init done");
}

unsigned long lastSpeedUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDepthUpdate = 0;
unsigned long lastGPSRead = 0;
unsigned long lastTaskEnd = 0;
unsigned long lastTaskStart = 0;
unsigned long lastTaskDuration = 0;
bool forceDispRefresh = true;
bool gpsWasRefreshed = false;

void startMeasure()
{
  lastTaskStart = millis();
}
void printMeasurement(const char *taskName)
{
  lastTaskEnd = millis();
  lastTaskDuration = lastTaskEnd - lastTaskStart;
  Serial.printf("%s took %lu milliseconds\n", taskName, lastTaskDuration);
  lastTaskStart = lastTaskEnd;
}

void loop()
{
  unsigned long now = millis();

  int clicks = buttonHandler.HandleClicks(now);
  if (clicks != 0)
  {
    dispHandler.HandleButtonInput(clicks);
    forceDispRefresh = true;
  }

  startMeasure();
  
  if (forceDispRefresh || now - lastGPSRead >= dispHandler.dispSettings.GPSUpdate)
  {

    if (gpsHandler.GetGps())
    {
      gpsWasRefreshed = true;
    }
    lastGPSRead = now;
    printMeasurement("GPSUpdate");
  }

  if (forceDispRefresh || now - lastSpeedUpdate >= dispHandler.dispSettings.speedRefreshTime)
  {

    dispHandler.DrawDisplay1(gpsHandler.stats, gpsHandler.lastNumOfSatellites, now);

    gpsWasRefreshed = false;

    lastSpeedUpdate = now;
    printMeasurement("Display1Update");
  }

  if (forceDispRefresh || now - lastDisplayUpdate >= dispHandler.dispSettings.fullRefreshTime)
  {
    dispHandler.DrawDisplay2(gpsHandler.lastNumOfSatellites, gpsHandler.stats, openEchoInterface.lastDepth);

    lastDisplayUpdate = now;

    printMeasurement("Display2Update");
  }

  // if (forceDispRefresh || now - lastDepthUpdate >= dispHandler.dispSettings.depthUpdate)
  // {
  //   Serial.println("ooecho");

  //   openEchoInterface.ReadPacket();

  //   lastDepthUpdate = now;

  //   printMeasurement("depthUpdate");
  // }

  if (forceDispRefresh)
  {
    forceDispRefresh = false;
  }

  wifiHandler.HandleRequests(dispHandler.dispSettings, gpsHandler.stats, &forceDispRefresh);
}
