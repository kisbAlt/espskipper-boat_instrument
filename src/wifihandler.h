#include <display.h>
class WifiHandler {
  private:
    void SplitQuery(String query);
  public:
    WifiHandler();
    void Init();
    void HandleRequests(const DisplaySettings& dispSettings);
};