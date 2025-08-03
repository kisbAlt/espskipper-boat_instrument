
#include <gpshandler.h>
#include <U8g2lib.h>

#define SPEED_HISTORY_COUNT 128
#define SPEED_GRAPH_HEIGHT 60

enum Language {
  ENGLISH = 0,
  HUNGARIAN = 1
};

enum DisplayState {
  SUMMARY = 0,
  DEPTH = 1,
  COURSE = 2,
  TEMP = 3,
  GYRO = 4,
  SPEED_AVG=5,
  SPEED_MAX=6,
  DISTANCE=7,
  SPEED_HISTORY=8
};
constexpr int maxDisplayState = 8; // or whatever the last value is

struct DisplaySettings
{
  bool useKnots = false;
  bool coursePartialUpdate = true;
  u_int32_t fullRefreshTime = 2000;
  u_int16_t speedRefreshTime = 1000;
  u_int16_t speedGraphUpdate = 5000;
  u_int16_t depthUpdate = 5000;
  int16_t depthOffset = 40; 
  Language language = ENGLISH;
  bool backlight_on = false;
  
  void SaveData();
  void LoadData();

};

struct StringTranslations {
  char AvgSpeed[15];
  char Course[15];
  char MaxSpeed[15];
  char Distance[15];
  char Kmph[15];
  char Knots[15];
  char Satellites[15];
  char WaterTemp[15];
  char Depth[15];
  char Kilometers[15];
  char Celsius[15];
  char Meters[15];
  char Degrees[15];
  char Interval[15];
  char Roll[15];
};

class DisplayHandler
{
private:
  U8G2_ST7920_128X64_F_SW_SPI display1;
  U8G2_ST7920_128X64_F_SW_SPI display2;
  void DrawLargeText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawMediumText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawSmallText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawUIBox();
  void PrepareDraw();
  void DrawSummary();
  void DrawGyro();
  void DrawSpeedHistory();
  const StringTranslations& getLangTranslations();
  char lastBuffer[10];
  u_int16_t lastCourse = 0;
  char satsBuf[40];
  char timeBuf[8];
  char speedBuffer[10];
  char rollBuf[10];
  int cpu_temp_celsius = 0;
  DisplayState display2State = SUMMARY;
  const char* GetSpeedUnitText();
  double speedHistory[SPEED_HISTORY_COUNT];
  uint8_t historyIndex = 0;
  void SaveSpeedHistory(double speed);
  bool speedHistoryUpdated = false;
  unsigned long lastHistoryUpdate = 0;
  u_int16_t numOfHistoryAvgCount = 0;
  double currentHistoryAvg = 1;
  

public:
  DisplayHandler();
  DisplaySettings dispSettings;
  void Init();
  void DrawDisplay2(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings, u_int16_t depth);
  void DrawDisplay1(BoatStats stats, u_int32_t satellites, unsigned long now);
  void HandleButtonInput(int clickCount);
};