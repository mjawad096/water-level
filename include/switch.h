#include <RCSwitch.h>
#include <setting.h>
#include <waterlevel.h>

#pragma once

class Switch
{
private:
    RCSwitch mySwitch;
    Setting *settings;

    int rfSwitchPin = GPIO_NUM_27;

    long lastSentOnTime = 0;
    long lastSentOffTime = 0;

    int previousLevel = 0;
    bool isDecreasing = false;
    int stopStateSentForLevel = -1;

public:
    void setup(Setting *settings)
    {
        this->settings = settings;

        mySwitch.enableTransmit(gpioNumberToDigitalPin(rfSwitchPin));
    }

    void determineSwitchState(WaterLevelData *levelData)
    {
        if (levelData == nullptr)
        {
            Serial.println("Error: Null water level data received.");
            return;
        }

        isDecreasing = levelData->level < previousLevel;

        checkForOpenState(levelData->level);
        checkForCloseState(levelData->level);

        previousLevel = levelData->level;
    }

    void sendSwitchState(bool state, bool manual = false)
    {
        int data = state ? 5557608 : 15859236;

        mySwitch.send(data, 24);
        delay(1000);

        mySwitch.send(data, 24);
        delay(1000);

        mySwitch.send(data, 24);

        if (manual)
        {
            return;
        }

        if (state)
        {
            lastSentOnTime = millis();
        }
        else
        {
            lastSentOffTime = millis();
        }
    }

    void checkForOpenState(int level)
    {
        if (!settings->autoOnOnEmpty)
        {
            return;
        }

        if (level > settings->emptyThreshold)
        {
            return;
        }

        if (millis() - lastSentOnTime < (settings->delayStartSwitch * 1000))
        {
            return;
        }

        sendSwitchState(true);
    }

    void checkForCloseState(int level)
    {
        if (!settings->autoOffOnFull || level < settings->fullThreshold || isDecreasing)
        {
            stopStateSentForLevel = -1;

            return;
        }

        if (stopStateSentForLevel == level || stopStateSentForLevel == level - 1)
        {
            return;
        }

        if (millis() - lastSentOffTime < (settings->delayStopSwitch * 1000))
        {
            return;
        }

        sendSwitchState(false);

        stopStateSentForLevel = level;
    }
};