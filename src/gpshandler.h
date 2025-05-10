#include <Arduino.h>

struct BoatStats
{
    double lastSpeedKmph = 0;
    double lastSpeedKt = 0;
    double avgSpeedKt = 0;
    double avgSpeedKmph = 0;
    
    double maxSpeedKmph = 0;
    double maxSpeedKt = 0;
    double lastLat = 0;
    double lastLng = 0;
    double lastCourse = 0;
};


class GpsHandler {
  private:
    void RefreshStats();
    u_int32_t avgNum;
  public:
    BoatStats stats;
    u_int32_t lastNumOfSatellites = 0;
    GpsHandler();
    void GetGps();
    void Init();
};