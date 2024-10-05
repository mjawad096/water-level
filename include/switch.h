#include <setting.h>
#include <waterlevel.h>
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

    static Switch *instance;

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

        instance = this;

        attachInterrupt(internalPin, handleInternalSwitchChange, CHANGE);
    }

    static void handleInternalSwitchChange()
    {
        int internalPinState = digitalRead(instance->internalPin);

        if (internalPinState == instance->internalPinState)
        {
            return;
        }

        instance->internalPinState = internalPinState;
        instance->changeSwitchState(!instance->currentSensor->isCurrentFlowing());
    }

    void changeSwitchState(bool state)
    {
        if (state != currentSensor->isCurrentFlowing())
        {
            buzzer->start(1, 300);

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
        }

        if (manualSwitchRequested != -1)
        {
            return;
        }
    }

    void handleSwitchState(WaterLevelData *levelData)
    {
        if (levelData == nullptr)
        {
            Serial.println("Error: Null water level data received.");
            return;
        }

        if (manualSwitchRequested != -1)
        {
            changeSwitchState(manualSwitchRequested ? true : false);

            manualSwitchRequested = -1;
        }
        else
        {
            checkForOpenState(levelData->level);
            checkForCloseState(levelData->level);
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

Switch *Switch::instance = nullptr;
RTC_DATA_ATTR int Switch::externalPinState = LOW;