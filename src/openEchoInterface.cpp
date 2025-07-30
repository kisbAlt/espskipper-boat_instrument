#include <openEchoInterface.h>
#include <Arduino.h>

#define NUM_SAMPLES 500
#include <HardwareSerial.h>

HardwareSerial ser(1);  // Use UART2

uint16_t samples[NUM_SAMPLES];
uint16_t depth;
float temperature, driveVoltage;
uint16_t* samplesOut;

bool OpenEchoInterface::ReadPacket() {
    ser.begin(250000, SERIAL_8N1, 4, -1);  // RX=GPIO4
    const uint8_t START_BYTE = 0xAA;
    const size_t PAYLOAD_SIZE = 6 + 2 * NUM_SAMPLES;
    const size_t TOTAL_PACKET_SIZE = 1 + PAYLOAD_SIZE + 1; // header + payload + checksum

    while (true) {
        if (ser.available() < TOTAL_PACKET_SIZE) {
            delay(10);  // Wait for more data
            continue;
        }

        uint8_t header = ser.read();
        Serial.println(header);
        if (header != START_BYTE) {
            continue;
        }

        uint8_t payload[PAYLOAD_SIZE];
        uint8_t checksum;

        // Read payload
        for (size_t i = 0; i < PAYLOAD_SIZE; ++i) {
            if (ser.available()) {
                payload[i] = ser.read();
            } else {
                return false; // Incomplete packet
            }
        }

        // Read checksum
        if (ser.available()) {
            checksum = ser.read();
        } else {
            return false; // Incomplete packet
        }

        // Compute checksum
        uint8_t calcChecksum = 0;
        for (size_t i = 0; i < PAYLOAD_SIZE; ++i) {
            calcChecksum ^= payload[i];
        }

        if (calcChecksum != checksum) {
            Serial.println("Checksum mismatch");
            continue;  // Try again
        }

        // Parse depth, temperature, drive voltage
        uint16_t depth = (payload[0] << 8) | payload[1];
        int16_t tempScaled = (payload[2] << 8) | payload[3];
        uint16_t vDrvScaled = (payload[4] << 8) | payload[5];

        depth = min(depth, (uint16_t)NUM_SAMPLES);
        int temperatureOut = tempScaled / 100.0;
        int driveVoltageOut = vDrvScaled / 100.0;
        int depthOut = depth;
        Serial.println(temperatureOut);
        Serial.println(driveVoltageOut);
        Serial.println(depthOut);
        // Parse samples
        for (size_t i = 0; i < NUM_SAMPLES; ++i) {
            size_t index = 6 + i * 2;
            samplesOut[i] = (payload[index] << 8) | payload[index + 1];
        }

        return true;  // Successfully read packet
    }
}