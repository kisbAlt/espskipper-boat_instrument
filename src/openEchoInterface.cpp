#include <openEchoInterface.h>
#include <Arduino.h>

#include <HardwareSerial.h>
#define READ_SAMPLES false
#define NUM_SAMPLES 1500
const uint8_t START_BYTE = 0xAA;
const size_t PAYLOAD_SIZE = 6 + 2 * NUM_SAMPLES;
const size_t TOTAL_PACKET_SIZE = 1 + PAYLOAD_SIZE + 1; // header + payload + checksum
uint16_t samples[NUM_SAMPLES];

HardwareSerial ser(1); // Use UART2

void OpenEchoInterface::Init()
{
    // needing hight buffer for 1800+ length
    ser.setRxBufferSize(4096);
    ser.begin(115200, SERIAL_8N1, 33, -1); // RX=GPI32
}

bool OpenEchoInterface::ReadPacket()
{
    if(!ser.available()){
        return false;
    }
    while (true)
    {
        uint8_t header = ser.read();;
        
        if (header == -1) {continue;} // No data

        if (header != START_BYTE) {continue;} // Not start byte


        if (header != START_BYTE)
        {
            continue;
        }

        uint8_t payload[PAYLOAD_SIZE];
        uint8_t checksum;

        if (!READ_SAMPLES)
        {
            while (ser.available() < 9)
            {
                delay(1);
            }
            size_t readCount = ser.readBytes(payload, 8);
        }
        else
        {
            // Wait until the rest of the packet is available
            while (ser.available() < TOTAL_PACKET_SIZE - 1)
            {
                delay(1);
            }
            // Read payload with timeout
            size_t readCount = ser.readBytes(payload, PAYLOAD_SIZE);
            if (readCount != PAYLOAD_SIZE)
            {
                Serial.println("readcount<PAYLOAD");
                return false; // Incomplete payload
            }

            // Read checksum byte
            readCount = ser.readBytes(&checksum, 1);
            if (readCount != 1)
            {
                Serial.println("missing checksum");
                return false; // Missing checksum
            }

            // Compute checksum
            uint8_t calcChecksum = 0;
            for (size_t i = 0; i < PAYLOAD_SIZE; ++i)
            {
                calcChecksum ^= payload[i];
            }

            if (calcChecksum != checksum)
            {
                Serial.println(calcChecksum);
                Serial.println(checksum);
                Serial.println("Checksum mismatch");
                // continue;  // Try again
            }
        }

        

        // // Read payload
        // for (size_t i = 0; i < PAYLOAD_SIZE; ++i)
        // {
        //     if (ser.available())
        //     {
        //         payload[i] = ser.read();
        //     }
        //     else
        //     {
        //         // continue;
        //         return false; // Incomplete packet
        //     }
        // }

        // // Read checksum
        // if (ser.available())
        // {
        //     checksum = ser.read();
        // }
        // else
        // {
        //     // continue;
        //     return false; // Incomplete packet
        // }

        // Parse depth, temperature, drive voltage
        uint16_t depth = (payload[0] << 8) | payload[1];
        int16_t tempScaled = (payload[2] << 8) | payload[3];
        uint16_t vDrvScaled = (payload[4] << 8) | payload[5];

        lastDepth = min(depth, (uint16_t)NUM_SAMPLES);
        int temperatureOut = tempScaled / 100.0;
        int driveVoltageOut = vDrvScaled / 100.0;
        // Serial.println("temp");
        // Serial.println(temperatureOut);
        // Serial.println("voltage");
        // Serial.println(driveVoltageOut);

        //Serial.println("depth");
        //Serial.println(lastDepth);

        // Parse samples
        // for (size_t i = 0; i < NUM_SAMPLES; ++i) {
        //     size_t index = 6 + i * 2;
        //     samplesOut[i] = (payload[index] << 8) | payload[index + 1];
        // }

        return true; // Successfully read packet
    }
}