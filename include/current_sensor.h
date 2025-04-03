#include "Esp.h"

#pragma once

class CurrentSensor
{
private:
    const int sensorPin = GPIO_NUM_33;

    const float R1 = 1000.0; // Resistor R1 in ohms (1KΩ)
    const float R2 = 1800.0; // Resistor R2 in ohms (1.8KΩ)

    // ACS712 parameters
    const float sensitivity = 0.066;  // Sensitivity for ACS712 30A version (66mV/A)
    const float noLoadVoltage = 2.18; // No-load voltage for ACS712 (2.5V)

    // Voltage conversion constants
    const float maxAnalogValue = 4095.0; // Maximum analog value
    const float referenceVoltage = 3.3;  // Maximum voltage for esp32

    float current = 0.0;   // Current calculated from sensor
    float threshold = 1.5; // Current threshold to be considered as flowing

public:
    CurrentSensor()
    {
    }

    float getCurrent()
    {
        float tempCurrent = 0.0;

        int analogValue = analogRead(sensorPin);

        if (analogValue != 0)
        {
            float voltage = analogValue * (referenceVoltage / maxAnalogValue);

            float inputVoltage = voltage * (R1 + R2) / R2;

            tempCurrent = (inputVoltage - noLoadVoltage) / sensitivity;

            // Serial.println(
            //     ", Analog Value:" + String(analogValue) +
            //     ", Voltage:" + String(voltage) +
            //     ", InputVoltage:" + String(inputVoltage) +
            //     ", Current:" + String(tempCurrent));
        }

        return tempCurrent;
    }

    void readCurrent()
    {
        float tempCurrent = 0.0;
        current = 0.0;

        for (int i = 0; i < 3; i++)
        {
            tempCurrent = abs(getCurrent());

            if (tempCurrent > current)
            {
                current = tempCurrent;
            }
        }

        // Serial.println("Current: " + String(current) + " A");
    }

    bool isCurrentFlowing()
    {
        return current > threshold;
    }
};
