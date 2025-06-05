#include <Arduino.h>
#include <wifihandler.h>

DisplayHandler dispHandler;
GpsHandler gpsHandler;
WifiHandler wifiHandler;

void setup() {
  Serial.begin(115200);
  delay(1000);
  dispHandler.Init();
  gpsHandler.Init();
  wifiHandler.Init();
  delay(1000);
}


unsigned long lastGpsPoll = 0;
unsigned long lastDisplayUpdate = 0;
bool firstRun = true;

void loop() {
  unsigned long now = millis();

  if (firstRun || now - lastDisplayUpdate >= dispHandler.dispSettings.fullRefreshTime) {
    float temp_celsius = temperatureRead();

    Serial.print("Temp onBoard ");
    Serial.print(temp_celsius);
    Serial.println("°C");
    
    Serial.println("Redrawing UI");
    dispHandler.DrawUI(gpsHandler.lastNumOfSatellites, gpsHandler.stats);
    lastDisplayUpdate = now;
  }

  if (firstRun || now - lastGpsPoll >= dispHandler.dispSettings.speedRefreshTime) {
    gpsHandler.GetGps();
    dispHandler.DrawSpeed(gpsHandler.stats.lastSpeedKmph);
    lastGpsPoll = now;
  }


  if(firstRun) {
    firstRun = false;
  }
  wifiHandler.HandleRequests(dispHandler.dispSettings);
}

