#include <Arduino.h>

#pragma once

class Distance
{
private:
    const int echoPin = D1;
    const int trigPin = D2;
    const double speedOfSound = 0.0343; // Speed of sound in cm/us

public:
    void
    setup()
    {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    unsigned long getDuration()
    {
        digitalWrite(trigPin, LOW);
        delay(100);

        digitalWrite(trigPin, HIGH);
        delayMicroseconds(30);

        digitalWrite(trigPin, LOW);

        return pulseIn(echoPin, HIGH);
    }

    double getBestDuration()
    {
        double duration;
        double maxDuration = 0;

        for (int i = 0; i < 3; i++)
        {
            duration = getDuration();

            Serial.print("Duration: ");
            Serial.println(duration);

            if (duration > maxDuration)
            {
                maxDuration = duration;
            }

            if (i < 2)
            {
                delay(1000);
            }
        }

        return maxDuration;
    }

    double getBestDistance()
    {
        double duration = getBestDuration();

        return (duration / 2.0) * speedOfSound;
    }
};