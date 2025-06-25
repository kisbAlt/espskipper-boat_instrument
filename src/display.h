#include <GxEPD2_BW.h>
#include <gpshandler.h>

enum Language {
  ENGLISH = 0,
  HUNGARIAN = 1
};

struct DisplaySettings
{
  bool useKnots = false;
  u_int32_t fullRefreshTime = 30000;
  u_int16_t speedRefreshTime = 2000;
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
  GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display;
  void DrawLargeText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawMediumText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawSmallText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawUIBox();
  StringTranslations getLangTranslations();

public:
  DisplayHandler();
  DisplaySettings dispSettings;
  void Init();
  void DrawUI(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings);
  void DrawSpeed(double speed);
};