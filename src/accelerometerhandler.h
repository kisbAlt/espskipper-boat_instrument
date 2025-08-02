class AccelerometerHandler {
public:
  float lastPitch = 0;
  float lastRoll = 0;
  float lastTemp = 0;
  AccelerometerHandler();
  void Init();
  void UpdateGyro();
};