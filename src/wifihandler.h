#include <display.h>
class WifiHandler {
  private:

  public:
    WifiHandler();
    void Init();
    void HandleRequests(const DisplaySettings& dispSettings);
};