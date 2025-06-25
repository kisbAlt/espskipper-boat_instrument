#include <gpshandler.h>
#include <Arduino.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART1
const float alpha = 0.1;

// Define the RX and TX pins
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

const float DISTANCE_THRESHOLD = 1.0; // meters

float haversine(float lat1, float lon1, float lat2, float lon2)
{
    const float R = 6371000; // Earth radius in meters
    float dLat = radians(lat2 - lat1);
    float dLon = radians(lon2 - lon1);

    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(radians(lat1)) * cos(radians(lat2)) *
                  sin(dLon / 2) * sin(dLon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

GpsHandler::GpsHandler()
{
}

void GpsHandler::Init()
{
    // Start Serial 2 with the defined RX and TX pins and a baud rate of 9600
    gpsSerial.end();
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

    Serial.println("Serial 2 started at 9600 baud rate");
}

void GpsHandler::RefreshStats()
{
    if (stats.lastSpeedKmph > stats.maxSpeedKmph)
    {
        stats.maxSpeedKmph = stats.lastSpeedKmph;
        stats.maxSpeedKt = stats.lastSpeedKt;
    }

    stats.avgSpeedKt = stats.avgSpeedKt + (stats.lastSpeedKt - stats.avgSpeedKt) / stats.numberOfSamples;
    stats.avgSpeedKmph = stats.avgSpeedKmph + (stats.lastSpeedKmph - stats.avgSpeedKmph) / stats.numberOfSamples;
    stats.numberOfSamples++;

    Serial.print("MaxKmph: ");
    Serial.println(stats.maxSpeedKmph);
    Serial.println();
}

bool GpsHandler::GetGps()
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

        if (stats.lastLat != 0)
        {
            float d = haversine(stats.lastLat, stats.lastLng, gps.location.lat(), gps.location.lng());
            if (d > DISTANCE_THRESHOLD)
            {
                stats.distance += d / 1000;
            }
        }

        if (firstRun)
        {
            stats.lastSpeedKmph = gps.speed.kmph();
            stats.lastSpeedKt = gps.speed.knots();
            firstRun = false;
        }
        else
        {
            stats.lastSpeedKmph = alpha * gps.speed.kmph() + (1 - alpha) * stats.lastSpeedKmph;
            stats.lastSpeedKt = alpha * gps.speed.knots() + (1 - alpha) * stats.lastSpeedKt;
        }

        // stats.lastSpeedKmph = gps.speed.kmph();
        // stats.lastSpeedKt = gps.speed.knots();
        lastNumOfSatellites = gps.satellites.value();
        stats.lastLat = gps.location.lat();
        stats.lastLng = gps.location.lng();
        stats.lastCourse = gps.course.deg();

        Serial.print("Lat: ");
        Serial.println(stats.lastLat);
        Serial.println();
        Serial.print("Lon: ");
        Serial.println(stats.lastLng);
        Serial.println();
        RefreshStats();
        return true;
    }
    else
    {
        Serial.println("GPS was not updated");
        return false;
    }
}