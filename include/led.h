#include <Arduino.h>

#pragma once

class Led
{
private:
    const int ON = HIGH;
    const int OFF = LOW;

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

    void on()
    {
        digitalWrite(ledPin, ON);
    }

    void on(bool stop)
    {
        if (stop)
        {
            this->stop();
        }

        on();
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
