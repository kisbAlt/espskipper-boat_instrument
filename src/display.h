
#include <gpshandler.h>
#include <U8g2lib.h>


enum Language {
  ENGLISH = 0,
  HUNGARIAN = 1
};

enum DisplayState {
  SUMMARY = 0,
  DEPTH = 1,
  COURSE = 2,
  TEMP = 3,
  SPEED_AVG=4,
  SPEED_MAX=5,
  DISTANCE=6,
};
constexpr int maxDisplayState = 6; // or whatever the last value is

struct DisplaySettings
{
  bool useKnots = false;
  bool coursePartialUpdate = true;
  u_int32_t fullRefreshTime = 2000;
  u_int16_t speedRefreshTime = 1000;
  Language language = HUNGARIAN;
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
  StringTranslations getLangTranslations();
  char lastBuffer[10];
  u_int16_t lastCourse = 0;
  char satsBuf[40];
  char timeBuf[8];
  char speedBuffer[10];
  int cpu_temp_celsius = 0;
  DisplayState display2State = SUMMARY;
  char* GetSpeedUnitText();


public:
  DisplayHandler();
  DisplaySettings dispSettings;
  void Init();
  void DrawDisplay2(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings);
  void DrawDisplay1(BoatStats stats, u_int32_t satellites);
  void HandleButtonInput(int clickCount);
};