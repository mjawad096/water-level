#pragma once

#include "Esp.h"
#include <logger.h>

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

    float current = 0;   // Current in Amperes
    float threshold = 2; // Current threshold to be considered as flowing

    // Noise filtering parameters
    const int numReadings = 20; // Number of readings for averaging
    float readings[20];         // Store last `numReadings` readings
    int readIndex = 0;          // Index for the current reading

public:
    CurrentSensor()
    {
        resetReadings(); // Initialize readings to zero
    }

    void resetReadings()
    {
        // Reset the readings array to zeros
        for (int i = 0; i < numReadings; i++)
        {
            readings[i] = 0.0;
        }
    }

    float calculateCurrent()
    {
        int analogValue = analogRead(sensorPin);

        if (analogValue != 0)
        {
            float voltage = analogValue * (referenceVoltage / maxAnalogValue);
            float inputVoltage = voltage * (R1 + R2) / R2;
            float tempCurrent = (inputVoltage - noLoadVoltage) / sensitivity;

            return abs(tempCurrent); // Return absolute value of current
        }

        return 0.0;
    }

    void readCurrent()
    {
        float tempCurrent = calculateCurrent(); // Get raw current reading

        // Track the maximum current from the last `numReadings` readings
        readings[readIndex] = tempCurrent; // Store the new reading

        readIndex = (readIndex + 1) % numReadings; // Move to the next index

        if (readIndex == 0)
        {
            current = getCurrent(); // Update current if we have cycled through all readings

            resetReadings(); // Reset readings if we have cycled through all

            // LOGL("Current: " + String(current) + " A");
        }
    }

    float getCurrent()
    {
        float maxReading = readings[0]; // Initialize maxReading with the first element

        // Iterate through the array to find the maximum value
        for (int i = 1; i < numReadings; i++)
        {
            if (readings[i] > maxReading)
            {
                maxReading = readings[i]; // Update maxReading if a higher value is found
            }
        }

        return maxReading; // Return the maximum value found
    }

    bool isCurrentFlowing()
    {
        return current > threshold;
    }
};
