#pragma once

#include <Arduino.h>

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

    // Dark means dark. The patterns go on running and keep their place --
    // they just do not light anything, so whatever was being shown is still
    // being shown when it comes back.
    bool dark = false;

public:
    Led(int pin = LED_BUILTIN) : ledPin(pin)
    {
    }

    void setup()
    {
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, OFF);
    }

    void on(bool stop)
    {
        if (stop)
        {
            this->stop();
        }

        on();
    }

    void setDark(bool value)
    {
        if (value == dark)
            return;

        dark = value;

        if (dark)
            off();
    }

    void on()
    {
        if (dark)
            return;

        digitalWrite(ledPin, ON);
    }

    void off()
    {
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

        off();
    }

    void stop()
    {
        blinking = false;

        off();
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

            ledState ? on() : off();

            lastToggleTime = millis();
        }
    }
};
