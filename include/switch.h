#include <RCSwitch.h>
#include <setting.h>

#pragma once

class Switch
{
private:
    RCSwitch mySwitch;
    Setting *settings;
    int rfSwitchPin = GPIO_NUM_27;
    long lastSentOnTime;
    long lastSentOffTime;

public:
    void setup(Setting *settings)
    {
        this->settings = settings;

        mySwitch.enableTransmit(gpioNumberToDigitalPin(rfSwitchPin));
    }

    void sendSwitchState(bool state)
    {
        int data = state ? 15859240 : 15859236;

        if (state)
        {
            lastSentOnTime = millis();
        }
        else
        {
            lastSentOffTime = millis();
        }

        mySwitch.send(data, 24);
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

        if (millis() - lastSentOnTime < (settings->delayStartSwitch * 60 * 1000))
        {
            return;
        }

        sendSwitchState(true);
    }

    void checkForCloseState(int level)
    {
        if (!settings->autoOffOnFull)
        {
            return;
        }

        if (level < settings->fullThreshold)
        {
            return;
        }

        if (millis() - lastSentOffTime < (settings->delayStopSwitch * 60 * 1000))
        {
            return;
        }

        sendSwitchState(false);
    }
};