#include <Arduino.h>

#pragma once

class Distance
{
private:
    const int echoPin = D1;
    const int trigPin = D2;
    const double speedOfSound = 0.0343; // Speed of sound in cm/us

public:
    void setup()
    {
        pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
        pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
    }

    unsigned long getDuration()
    {
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);

        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);

        digitalWrite(trigPin, LOW);

        return pulseIn(echoPin, HIGH);
    }

    double getBestDuration()
    {
        double durationSum = 0;
        int nb_measurements = 10;

        for (int i = 0; i < nb_measurements; i++)
        {
            durationSum += getDuration();
        }

        return durationSum / nb_measurements;
    }

    double getBestDistance()
    {
        double duration = getBestDuration();

        return (duration / 2.0) * speedOfSound;
    }
};