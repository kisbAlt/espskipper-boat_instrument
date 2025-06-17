#include <GxEPD2_BW.h>
#include <gpshandler.h>

struct DisplaySettings
{
  bool useKnots = false;
  u_int32_t fullRefreshTime = 30000;
  u_int16_t speedRefreshTime = 2000;
};

class DisplayHandler
{
private:
  GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display;
  void DrawLargeText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawMediumText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawSmallText(char text[], int16_t x, int16_t y, bool centerX);
  void DrawUIBox();

public:
  DisplayHandler();
  DisplaySettings dispSettings;
  void Init();
  void DrawUI(u_int32_t satellites, BoatStats stats, DisplaySettings displaySettings);
  void DrawSpeed(double speed);
};