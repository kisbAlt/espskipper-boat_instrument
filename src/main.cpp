#include <Arduino.h>
#include <wifihandler.h>

DisplayHandler dispHandler;
GpsHandler gpsHandler;
WifiHandler wifiHandler;

void findFirstGps() {
  
  while (gpsHandler.stats.lastLat == 0){
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
  findFirstGps();

  wifiHandler.Init();
  

}

unsigned long lastGpsPoll = 0;
unsigned long lastDisplayUpdate = 0;
bool firstRun = true;

void loop()
{
  unsigned long now = millis();

  if (firstRun || now - lastDisplayUpdate >= dispHandler.dispSettings.fullRefreshTime)
  {
    float temp_celsius = temperatureRead();

    Serial.print("Temp onBoard ");
    Serial.print(temp_celsius);
    Serial.println("°C");

    Serial.println("Redrawing UI");
    dispHandler.DrawUI(gpsHandler.lastNumOfSatellites, gpsHandler.stats, dispHandler.dispSettings);
    lastDisplayUpdate = now;
  }

  if (firstRun || now - lastGpsPoll >= dispHandler.dispSettings.speedRefreshTime)
  {
    gpsHandler.GetGps();
    if (dispHandler.dispSettings.useKnots)
    {
      dispHandler.DrawSpeed(gpsHandler.stats.lastSpeedKt);
    }
    else
    {
      dispHandler.DrawSpeed(gpsHandler.stats.lastSpeedKmph);
    }
    lastGpsPoll = now;
  }

  if (firstRun)
  {
    firstRun = false;
  }
  wifiHandler.HandleRequests(dispHandler.dispSettings, gpsHandler.stats);
}
