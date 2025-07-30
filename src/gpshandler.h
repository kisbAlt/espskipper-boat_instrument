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
  uint8_t lastHour = 0;
  uint8_t lastMinute = 0;

  u_int16_t lastCourse = 0;
  u_int32_t numberOfSamples = 1;

  void Reset()
  {
    avgSpeedKmph = 0;
    avgSpeedKt = 0;
    distance = 0;
    maxSpeedKmph = 0;
    maxSpeedKt = 0;
    numberOfSamples = 1;
  };
};

class GpsHandler
{
private:
  void RefreshStats();
  bool firstRun = true;

public:
  BoatStats stats;
  u_int32_t lastNumOfSatellites = 0;
  GpsHandler();
  bool GetGps();
  void Init();
};