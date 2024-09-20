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

    int previousLevel = 0;
    bool isDecreasing = false;
    int stopStateSentForLevel = -1;

    int manualSwitchRequested = -1;

public:
    void setup(Setting *settings)
    {
        this->settings = settings;

        mySwitch.enableTransmit(gpioNumberToDigitalPin(rfSwitchPin));
    }

    void handleSwitchState(WaterLevelData *levelData)
    {
        if (levelData == nullptr)
        {
            Serial.println("Error: Null water level data received.");
            return;
        }

        isDecreasing = levelData->level < previousLevel;

        if (manualSwitchRequested != -1)
        {
            sendSwitchState(manualSwitchRequested ? true : false);
        }
        else
        {
            checkForOpenState(levelData->level);
            checkForCloseState(levelData->level);
        }

        previousLevel = levelData->level;
    }

    void sendSwitchState(bool state)
    {
        int data = state ? 5557608 : 15859236;

        mySwitch.send(data, 24);
        delay(2000);

        mySwitch.send(data, 24);
        delay(2000);

        mySwitch.send(data, 24);

        if (manualSwitchRequested != -1)
        {
            manualSwitchRequested = -1;
            return;
        }

        if (state)
        {
            lastSentOnTime = millis();
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

        sendSwitchState(true);
    }

    void checkForCloseState(int level)
    {
        if (!settings->autoOffOnFull || level < settings->fullThreshold || isDecreasing)
        {
            stopStateSentForLevel = -1;

            return;
        }

        if (stopStateSentForLevel == level)
        {
            return;
        }

        sendSwitchState(false);

        stopStateSentForLevel = level;
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