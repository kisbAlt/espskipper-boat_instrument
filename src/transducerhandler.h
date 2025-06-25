// Airmar thermister equation coefficients
#define A 0.0011321253
#define B 0.000233722
#define C 0.0000000886

#define ANALOG_READ_REFERENCE_VOLTAGE 3.3 // esp32 uses 3.3v instead of 5
#define ANALOG_READ_RANGE 4095.0 //ESP32 uses 12 bit so no 1023

class TransducerHandler {
    int ReadTemp();
};