#include <display.h>
class WifiHandler {
  private:
    void ParseAndUpdateSettings(String query, DisplaySettings& settings);
  public:
    WifiHandler();
    void Init();
    void HandleRequests(DisplaySettings& dispSettings, BoatStats &boatStats);
};