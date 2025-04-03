#include <setting.h>
#include "current_sensor.h"
#include "buzzer.h"

#pragma once

class Switch
{
private:
    Setting *settings;
    CurrentSensor *currentSensor;
    Buzzer *buzzer;

    int internalPinState = LOW;

    const int externalPin = GPIO_NUM_27;
    const int internalPin = GPIO_NUM_32;

    long lastSentOnTime = 0;
    long lastSentOffTime = 0;

    int manualSwitchRequested = -1;

    // Debounce timing variables for interal pin
    long lastDebounceTime = 0;
    const long debounceDelay = 100;

public:
    static RTC_DATA_ATTR int externalPinState;

    void setup(Setting *settings, CurrentSensor *currentSensor, Buzzer *buzzer)
    {
        this->settings = settings;
        this->currentSensor = currentSensor;
        this->buzzer = buzzer;

        pinMode(externalPin, OUTPUT);
        pinMode(internalPin, INPUT_PULLUP);

        internalPinState = digitalRead(internalPin);
        digitalWrite(externalPin, externalPinState);
    }

    void checkForInternalSwitchChange()
    {
        int newInternalPinState = digitalRead(internalPin);

        if (internalPinState == newInternalPinState)
        {
            return;
        }

        long currentTime = millis();
        if (currentTime - lastDebounceTime <= debounceDelay)
        {
            return;
        }

        lastDebounceTime = currentTime;

        internalPinState = newInternalPinState;

        Serial.print("Internal switch state changed: ");
        Serial.println(internalPinState);

        changeSwitchState(!currentSensor->isCurrentFlowing());
    }

    void changeSwitchState(bool state)
    {
        if (manualSwitchRequested != -1)
        {
            Serial.println("Manual switch requested: " + String(manualSwitchRequested));
        }

        if (state != currentSensor->isCurrentFlowing())
        {
            buzzer->start(1, 100);

            int newExternalPinState = externalPinState == HIGH ? LOW : HIGH;

            digitalWrite(externalPin, newExternalPinState);

            externalPinState = newExternalPinState;

            if (state)
            {
                lastSentOnTime = millis();
            }
            else
            {
                lastSentOffTime = millis();
            }

            Serial.println("Switch State changed to " + String(state));
        }
        else
        {
            Serial.println("Switch is already in the requested state.");
        }
    }

    void handleSwitchState(int level)
    {
        if (manualSwitchRequested != -1)
        {
            changeSwitchState(manualSwitchRequested ? true : false);

            manualSwitchRequested = -1;
        }
        else
        {
            checkForOpenState(level);
            checkForCloseState(level);
        }
    }

    void checkForOpenState(int level)
    {
        if (!settings->autoOnOnEmpty || level > settings->emptyThreshold)
        {
            return;
        }

        if (millis() - lastSentOnTime < (settings->delayStartSwitch * 1000))
        {
            return;
        }

        changeSwitchState(true);
    }

    void checkForCloseState(int level)
    {
        if (!settings->autoOffOnFull || level < settings->fullThreshold)
        {
            return;
        }

        if (millis() - lastSentOffTime < (settings->delayStopSwitch * 1000))
        {
            return;
        }

        changeSwitchState(false);
    }

    void manualStart()
    {
        manualSwitchRequested = 1;
    }

    void manualStop()
    {
        manualSwitchRequested = 0;
    }
};

RTC_DATA_ATTR int Switch::externalPinState = LOW;