#include <accelerometerhandler.h>
#include <Arduino.h>
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"

#define LIS3DH_CS 22
LIS3DH SensorOne(SPI_MODE, 22);

AccelerometerHandler::AccelerometerHandler()
{
}

void AccelerometerHandler::Init()
{
    Serial.println("Init lis3dh");
    delay(1000); // relax...
    Serial.println("Processor came out of reset.\n");

    SensorOne.settings.tempEnabled = 1;
    SensorOne.settings.accelSampleRate = 10; // Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
    SensorOne.settings.accelRange = 2;       // Max G force readable.  Can be: 2, 4, 8, 16

    // Call .begin() to configure the IMUs
    uint8_t returnData = 0;
    SPI.begin();
    returnData = SensorOne.begin();
    if ((returnData != 0x00) && (returnData != 0xFF))
    {
        Serial.println("Problem starting the sensor with CS @ Pin 22.");
    }
    else
    {
        Serial.println("Sensor with CS @ Pin 22 started.");
    }
}

void AccelerometerHandler::UpdateGyro()
{
    SPI.beginTransaction(SPISettings(530000, MSBFIRST, SPI_MODE0));
    digitalWrite(LIS3DH_CS, LOW);

    accelx = SensorOne.readFloatAccelX();
    accely = SensorOne.readFloatAccelY();
    accelz = SensorOne.readFloatAccelZ();
    digitalWrite(LIS3DH_CS, HIGH);
    SPI.endTransaction();

    lastPitch = -atan2(accelx / 9.8, accelz / 9.8) / 2 / 3.141592654 * 360;
    lastRoll = -atan2(accely / 9.8, accelz / 9.8) / 2 / 3.141592654 * 360;

    // Get all parameters
    Serial.print("\nAccelerometer:\n");
    // Serial.print(" X1 = ");
    // Serial.println(accelx, 4);
    // Serial.print(" Y1 = ");
    // Serial.println(accely, 4);
    // Serial.print(" Z1 = ");
    // Serial.println(accelz, 4);

    Serial.print(" lastPitch = ");
    Serial.println(lastPitch);
    Serial.print(" lastRoll = ");
    Serial.println(lastRoll);

    // int16_t temp = ((int16_t)SensorOne.read10bitADC3()) / 256;
    // temp += 25;
    // Serial.print(" TEMP = ");
    // Serial.println(temp);

    // Serial.print("\nSensorOne Bus Errors Reported:\n");
    // Serial.print(" All '1's = ");
    // Serial.println(SensorOne.allOnesCounter);
    // Serial.print(" Non-success = ");
    // Serial.println(SensorOne.nonSuccessCounter);
}
