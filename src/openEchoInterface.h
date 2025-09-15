#pragma once

#include <Arduino.h>



class OpenEchoInterface {
    private:
    public:
    bool ReadPacket();
    void Init();
    u_int16_t lastDepth = 0;
    float temperature, driveVoltage;
    uint16_t *samplesOut;
    
};