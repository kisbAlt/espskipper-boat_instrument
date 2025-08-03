#include <openEchoInterface.h>
#include <Arduino.h>

#include <HardwareSerial.h>
#define READ_SAMPLES false
#define NUM_SAMPLES 1500
#define SERIAL_BUFFER 4096
const uint8_t START_BYTE = 0xAA;
const size_t PAYLOAD_SIZE = 6 + 2 * NUM_SAMPLES;
const size_t TOTAL_PACKET_SIZE = 1 + PAYLOAD_SIZE + 1; // header + payload + checksum
uint16_t samples[NUM_SAMPLES];

HardwareSerial ser(1); // Use UART2

void OpenEchoInterface::Init()
{
    // needing hight buffer for 1800+ length
    ser.setRxBufferSize(SERIAL_BUFFER);
    ser.begin(115200, SERIAL_8N1, 33, -1); // RX=GPI32
}

bool OpenEchoInterface::ReadPacket()
{
    if (!ser.available())
    {
        return false;
    }
    uint8_t readBuf[SERIAL_BUFFER];
    size_t byteCount = ser.readBytes(readBuf, ser.available() - 1);
    int byteIndex = 0;
    while (byteIndex < byteCount)
    {
        if (readBuf[byteIndex] != START_BYTE)
        {
            byteIndex = byteIndex + 1;
            continue;
        }

        // skipping the START_BYTE byte
        byteIndex = byteIndex + 1;

        // If not enough data in buffer, return
        if (byteIndex > byteCount - 10)
        {
            return false;
        }

        uint8_t payload[PAYLOAD_SIZE];
        uint8_t checksum;

        if (READ_SAMPLES)
        {
            // TODO: INCOMPLETE IMPLEMENTATION

            if (byteIndex > byteCount-TOTAL_PACKET_SIZE)
            {
                return false;
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

        uint16_t depth = (readBuf[byteIndex] << 8) | readBuf[byteIndex + 1];
        int16_t tempScaled = (readBuf[byteIndex + 2] << 8) | readBuf[byteIndex + 3];
        uint16_t vDrvScaled = (readBuf[byteIndex + 4] << 8) | readBuf[byteIndex + 5];

        lastDepth = min(depth, (uint16_t)NUM_SAMPLES);
        int temperatureOut = tempScaled / 100.0;
        int driveVoltageOut = vDrvScaled / 100.0;

        Serial.println("depth1:");
        Serial.println(lastDepth);

        return true; // Successfully read packet
    }
    return false;
}