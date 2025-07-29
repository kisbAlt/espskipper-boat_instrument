
#include <gpshandler.h>
#include <U8g2lib.h>


enum Language {
  ENGLISH = 0,
  HUNGARIAN = 1
};

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
  void DrawDisplay1();
  StringTranslations getLangTranslations();
  char lastBuffer[10];
  u_int16_t lastCourse = 0;
  char satsBuf[40];
  bool prepared = false;

public:
  DisplayHandler();
  DisplaySettings dispSettings;
  void Init();
  void DrawUI(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings);
  void DrawSpeed(double speed);
  void PartialCourse(u_int16_t course);
};