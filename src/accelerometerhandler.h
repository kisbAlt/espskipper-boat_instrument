#pragma once

class AccelerometerHandler
{
private:
  float accelx;
  float accely;
  float accelz;

public:
  float lastPitch = 0;
  float lastRoll = 0;
  float lastTemp = 0;
  AccelerometerHandler();
  void Init();
  void UpdateGyro();
};