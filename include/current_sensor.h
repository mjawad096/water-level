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
    float threshold = 3; // Current threshold to be considered as flowing

    // Noise filtering parameters
    const int numReadings = 20; // Number of readings for averaging
    float readings[20];         // Store last `numReadings` readings
    int readIndex = 0;          // Index for the current reading

    const int numMaxPasses = 3; // Number of passes for max value
    float passMaxValues[3];     // Stores max value from each pass
    int passIndex = 0;          // Which pass we are on

public:
    CurrentSensor()
    {
        resetReadings();      // Initialize readings to zero
        resetPassMaxValues(); // Initialize pass max values to zero
    }

    void resetReadings()
    {
        // Reset the readings array to zeros
        for (int i = 0; i < numReadings; i++)
        {
            readings[i] = 0.0;
        }
    }

    void resetPassMaxValues()
    {
        // Reset the pass max values array to zeros
        for (int i = 0; i < numMaxPasses; i++)
        {
            passMaxValues[i] = 0.0;
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

        // If we filled one pass of 20 readings
        if (readIndex == 0)
        {
            float maxVal = getMaxCurrentReading(); // Max from current pass

            passMaxValues[passIndex] = maxVal;

            passIndex = (passIndex + 1) % numMaxPasses; // Move to the next pass

            // After 3 passes, update current with the min of 3 max values
            if (passIndex == 0)
            {
                filterFinalCurrent(); // Filter the final current value

                // LOGL("Current: " + String(current) + " A");
            }
        }
    }

    float getMaxCurrentReading()
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

        resetReadings(); // Reset readings for the next pass

        return maxReading; // Return the maximum value found
    }

    void filterFinalCurrent()
    {
        // Start with the first pass's max value
        float finalFiltered = passMaxValues[0];

        // Compare it with the next two passes
        for (int i = 1; i < numMaxPasses; i++)
        {
            if (passMaxValues[i] < finalFiltered)
            {
                finalFiltered = passMaxValues[i];
            }
        }

        // Update the current value with the final filtered value
        current = finalFiltered;

        resetPassMaxValues(); // Reset pass max values for the next cycle
    }

    float getCurrent()
    {
        return current; // Return the current value
    }

    bool isCurrentFlowing()
    {
        return current > threshold;
    }
};
