#include <gpshandler.h>
#include <Arduino.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART1

// Define the RX and TX pins
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

GpsHandler::GpsHandler()
{
    avgNum = 1;
}

void GpsHandler::Init()
{
    // Start Serial 2 with the defined RX and TX pins and a baud rate of 9600
    gpsSerial.end();
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
    // gpsSerial.begin(9600);
    Serial.println("Serial 2 started at 9600 baud rate");
}

void GpsHandler::RefreshStats() {
    if (stats.lastSpeedKmph > stats.maxSpeedKmph) {
        stats.maxSpeedKmph = stats.lastSpeedKmph;
        stats.maxSpeedKt = stats.lastSpeedKt;
    }
    
    stats.avgSpeedKt = stats.avgSpeedKt+ (stats.lastSpeedKt-stats.avgSpeedKt) / avgNum;
    stats.avgSpeedKmph = stats.avgSpeedKmph+ (stats.lastSpeedKmph-stats.avgSpeedKmph) / avgNum;
    avgNum++;

    Serial.print("MaxKmph: ");
    Serial.println(stats.maxSpeedKmph);
    Serial.println();
}

void GpsHandler::GetGps()
{
    while (gpsSerial.available())
    {
        gps.encode(gpsSerial.read());
    }

    if (gps.location.isUpdated())
    {
        Serial.print("Satellites: ");
        Serial.println(gps.satellites.value());
        Serial.println();
        Serial.print("Speed km: ");
        Serial.println(gps.speed.kmph());
        Serial.println();
        
        stats.lastSpeedKmph = gps.speed.kmph();
        stats.lastSpeedKt = gps.speed.knots();
        lastNumOfSatellites = gps.satellites.value();
        stats.lastLat = gps.location.lat();
        stats.lastLng = gps.location.lng();
        stats.lastCourse = gps.course.deg();
    }
    RefreshStats();
}