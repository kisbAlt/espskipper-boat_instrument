#include <Arduino.h>

struct BoatStats
{
    double lastSpeedKmph = 0;
    double lastSpeedKt = 0;
    double avgSpeedKt = 0;
    double avgSpeedKmph = 0;
    double distance = 0;

    double maxSpeedKmph = 0;
    double maxSpeedKt = 0;
    double lastLat = 0;
    double lastLng = 0;
    u_int16_t lastCourse = 0;
    u_int32_t numberOfSamples = 1;
};


class GpsHandler {
  private:
    void RefreshStats();
  public:
    BoatStats stats;
    u_int32_t lastNumOfSatellites = 0;
    GpsHandler();
    void GetGps();
    void Init();
};