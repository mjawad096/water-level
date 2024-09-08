#include <RCSwitch.h>

#pragma once

class Switch
{
private:
    RCSwitch mySwitch;
    int rfSwitchPin = GPIO_NUM_27;

public:
    Switch()
    {
        mySwitch.enableTransmit(gpioNumberToDigitalPin(rfSwitchPin));
    }

    void sendSwitchState(bool state)
    {
        int data = state ? 15859240 : 15859236;

        mySwitch.send(data, 24);
    }
};