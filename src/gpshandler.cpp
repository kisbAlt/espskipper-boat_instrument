#include <gpshandler.h>
#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <MicroNMEA.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART1
const float alpha = 0.1;

// Define the RX and TX pins
#define RXD2 35
#define TXD2 13
#define GPS_BAUD_SLOW 9600
#define GPS_BAUD 115200
#define GPS_BUFFER 2048

#define FORCE_ON_PIN 12

// If the dist between two GPS points is less than this value it wont be added to the distance travelled
const float DISTANCE_THRESHOLD = 1.0; // meters


char microbuffer[85];
MicroNMEA microgps(microbuffer, sizeof(microbuffer));

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
    gpsSerial.begin(GPS_BAUD_SLOW, SERIAL_8N1, RXD2, TXD2);
    delay(2000);

    // Send command to change baud to 115200
    gpsSerial.println("$PMTK251,115200*1F");

    delay(500);

    // Reinitialize Arduino serial to new baud
    gpsSerial.end();

    delay(500);
    gpsSerial.setRxBufferSize(GPS_BUFFER);
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

    digitalWrite(FORCE_ON_PIN, HIGH); // force module on
    delay(2000);

    gpsSerial.println("$PMTK225,0*2B");

    delay(100);

    gpsSerial.println("$PMTK353,1,1,1,0,0*2A");


    // Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
    // Serial.println("Serial 2 started at 9600 baud rate");
    // while (true)
    // {
    //     if (Serial2.available())
    //     {
    //         char c = Serial2.read();
    //         Serial.write(c);
    //     }
    //     else
    //     {
    //         // Debug: show we're checking
    //         // (don't leave this forever, just for testing)
    //         // Serial.println("No data from L96...");
    //         delay(500);
    //     }
    // }
}

void GpsHandler::RefreshStats()
{
    if (stats.lastSpeedKmph > stats.maxSpeedKmph)
    {
        stats.maxSpeedKmph = stats.lastSpeedKmph;
    }

    // calculating the avg speed iteratively
    stats.avgSpeedKmph = stats.avgSpeedKmph + (stats.lastSpeedKmph - stats.avgSpeedKmph) / stats.numberOfSamples;
    stats.numberOfSamples++;

    // Serial.print("MaxKmph: ");
    // Serial.println(stats.maxSpeedKmph);
    // Serial.println();
}

bool GpsHandler::GetGps()
{
    while (gpsSerial.available())
    {
        char gpsRead = gpsSerial.read();
        Serial.write(gpsRead);
        gps.encode(gpsRead);
        microgps.process(gpsRead);
    }
    if (gps.location.isUpdated())
    {
        Serial.print("Satellites: ");
        Serial.println(gps.satellites.value());
        Serial.println();
        Serial.print("Speed km: ");
        Serial.println(gps.speed.kmph());

        Serial.print("Speed micro: ");
        Serial.println(microgps.getSpeed() / 1000., 3);
        Serial.print("Satellites micro: ");
        Serial.println(microgps.getNumSatellites() / 1000., 3);
        Serial.print("navsystem micro: ");
        if (microgps.getNavSystem())
            Serial.println(microgps.getNavSystem());
        else
            Serial.println("none");

        Serial.print("HDOP: ");
        Serial.println(microgps.getHDOP() / 10., 1);
        microgps.clear();

        Serial.println();

        // Ignore the first time it run by checking lastLat value
        if (stats.lastLat != 0)
        {
            float d = haversine(stats.lastLat, stats.lastLng, gps.location.lat(), gps.location.lng());
            if (d > DISTANCE_THRESHOLD && d / 1000 < 0.5)
            {
                stats.distance += d / 1000;
            }
        }

        if (firstRun)
        {
            stats.lastSpeedKmph = gps.speed.kmph();
            firstRun = false;
        }
        else
        {
            // adding filtering for the speed data
            stats.lastSpeedKmph = alpha * gps.speed.kmph() + (1 - alpha) * stats.lastSpeedKmph;
        }

        lastNumOfSatellites = gps.satellites.value();
        stats.lastLat = gps.location.lat();
        stats.lastLng = gps.location.lng();
        stats.lastCourse = gps.course.deg();
        stats.lastHour = gps.time.hour();
        stats.lastMinute = gps.time.minute();

        // Serial.print("Lat: ");
        // Serial.println(stats.lastLat);
        // Serial.println();
        // Serial.print("Lon: ");
        // Serial.println(stats.lastLng);
        // Serial.println();
        // Serial.print("couse inner: ");
        // Serial.println(gps.course.deg());
        // Serial.println();
        RefreshStats();
        return true;
    }
    else
    {
        // Serial.println("GPS was not updated");
        return false;
    }
}

double BoatStats::GetLastSpeed(const bool &useKnots) const
{
    if (useKnots)
    {
        return ConvertToKnots(lastSpeedKmph);
    }
    return lastSpeedKmph;
}
double BoatStats::GetMaxSpeed(const bool &useKnots) const
{
    if (useKnots)
    {
        return ConvertToKnots(maxSpeedKmph);
    }
    return maxSpeedKmph;
}
double BoatStats::GetAvgSpeed(const bool &useKnots) const
{
    if (useKnots)
    {
        return ConvertToKnots(avgSpeedKmph);
    }
    return avgSpeedKmph;
}

double BoatStats::ConvertToKnots(const double &kmph)
{
    return kmph / 1.852;
}
