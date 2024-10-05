#pragma once
#include <Arduino.h>

class Buzzer
{
private:
    int buzzerPin;
    unsigned long buzzerStartTime; // Time when the buzzer was started
    unsigned long buzzerDuration;  // Total duration for how long the buzzer runs
    unsigned long cycleInterval;   // Interval for on/off cycling in ms
    unsigned long lastToggleTime;  // Time of the last buzzer state toggle
    bool cycleMode;                // Indicates if it's cycling mode (on/off)
    bool buzzerActive;             // Track if the buzzer is active
    bool buzzerState;              // Current state of the buzzer (on/off)
    int uniqueId;                  // Unique ID for the buzzer

public:
    Buzzer(int pin = GPIO_NUM_4) : buzzerPin(pin)
    {
    }

    void setup()
    {
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW); // Ensure the buzzer is off initially

        buzzerStartTime = 0;
        buzzerDuration = 0;
        cycleInterval = 0;
        lastToggleTime = 0;
        cycleMode = false;
        buzzerActive = false;
        buzzerState = false;
        uniqueId = -1;
    }

    void start(int uniqueId, unsigned long duration, unsigned long cycleTime = 0)
    {
        if (buzzerActive && this->uniqueId == uniqueId)
        {
            return; // Do not start the buzzer if it's already active
        }

        this->uniqueId = uniqueId;

        buzzerStartTime = millis(); // Record the start time
        buzzerDuration = duration;  // Set the total duration for the buzzer
        cycleInterval = cycleTime;  // Set the cycle time for on/off toggling
        lastToggleTime = 0;         // Reset toggle time

        cycleMode = (cycleTime > 0);
        buzzerActive = true; // Mark the buzzer as active
        buzzerState = true;  // Start with the buzzer on

        digitalWrite(buzzerPin, HIGH);
    }

    void stop(bool reset = false)
    {
        digitalWrite(buzzerPin, LOW); // Turn off the buzzer

        if (!reset)
        {
            return;
        }

        buzzerActive = false; // Mark the buzzer as inactive
        uniqueId = -1;        // Reset the unique ID
    }

    void update()
    {
        if (!buzzerActive)
        {
            return;
        }

        if ((millis() - buzzerStartTime) >= buzzerDuration)
        {
            stop(); // Turn off the buzzer after the set duration
            return;
        }

        if (cycleMode)
        {
            if ((millis() - lastToggleTime) >= cycleInterval)
            {
                // Toggle buzzer state (on/off) based on cycle interval
                buzzerState = !buzzerState;
                digitalWrite(buzzerPin, buzzerState ? HIGH : LOW);

                lastToggleTime = millis(); // Reset toggle time
            }
        }
    }
};
