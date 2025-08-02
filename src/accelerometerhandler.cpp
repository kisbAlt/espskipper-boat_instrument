#include <accelerometerhandler.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define SDA_PIN 25
#define SCL_PIN 26


TwoWire I2C_MPU = TwoWire(1);  // Create a second I²C bus
Adafruit_MPU6050 mpu;

AccelerometerHandler::AccelerometerHandler()
{
}

void AccelerometerHandler::Init()
{
    I2C_MPU.begin(SDA_PIN, SCL_PIN);
    if (!mpu.begin(0x68, &I2C_MPU))
    {
        Serial.println("Failed to find MPU6050 chip");
        while (1)
        {
            delay(10);
        }
    }
    Serial.println("MPU6050 Found!");

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    Serial.print("Accelerometer range set to: ");
    switch (mpu.getAccelerometerRange())
    {
    case MPU6050_RANGE_2_G:
        Serial.println("+-2G");
        break;
    case MPU6050_RANGE_4_G:
        Serial.println("+-4G");
        break;
    case MPU6050_RANGE_8_G:
        Serial.println("+-8G");
        break;
    case MPU6050_RANGE_16_G:
        Serial.println("+-16G");
        break;
    }
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    Serial.print("Gyro range set to: ");
    switch (mpu.getGyroRange())
    {
    case MPU6050_RANGE_250_DEG:
        Serial.println("+- 250 deg/s");
        break;
    case MPU6050_RANGE_500_DEG:
        Serial.println("+- 500 deg/s");
        break;
    case MPU6050_RANGE_1000_DEG:
        Serial.println("+- 1000 deg/s");
        break;
    case MPU6050_RANGE_2000_DEG:
        Serial.println("+- 2000 deg/s");
        break;
    }

    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

}

void AccelerometerHandler::UpdateGyro() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    Serial.print("Temperature: ");
    Serial.print(temp.temperature);
    Serial.println(" degC");
    lastTemp = temp.temperature;

    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    lastRoll = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
    lastPitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

    Serial.print("Pitch: "); Serial.print(lastPitch);
    Serial.print(" Roll: "); Serial.println(lastRoll);
}