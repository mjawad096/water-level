#include <Arduino.h>

#pragma once

class Led
{
private:
    const int ON = LOW;
    const int OFF = HIGH;

    int ledPin;

    unsigned long blinkInterval;
    unsigned long lastToggleTime;

    bool ledState;
    bool blinking;

public:
    Led(int pin = LED_BUILTIN) : ledPin(pin)
    {
    }

    void setup()
    {
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, OFF);
    }

    void blinkFor(unsigned long interval)
    {
        blinkInterval = interval;

        if (blinking)
        {
            return;
        }

        lastToggleTime = millis();
        ledState = false;
        blinking = true;

        digitalWrite(ledPin, OFF);
    }

    void stop()
    {
        blinking = false;

        digitalWrite(ledPin, OFF);
    }

    void blink()
    {
        if (!blinking)
        {
            return;
        }

        if (millis() - lastToggleTime >= blinkInterval)
        {
            ledState = !ledState;

            digitalWrite(ledPin, ledState ? HIGH : LOW);

            lastToggleTime = millis();
        }
    }
};
