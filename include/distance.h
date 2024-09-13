#include <Arduino.h>

#pragma once

class Distance
{
private:
    const int echoPin = D1;
    const int trigPin = D2;
    const double speedOfSound = 0.0343;

public:
    void setup()
    {
        pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
        pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
    }

    double readDistance()
    {
        digitalWrite(trigPin, LOW);
        delay(100);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(30);
        digitalWrite(trigPin, LOW);

        long duration = pulseIn(echoPin, HIGH);

        return duration * speedOfSound / 2; // cm
    }

    double getBestDistance()
    {
        double distance;
        double maxDistance = 0;

        for (int i = 0; i < 3; i++)
        {
            distance = readDistance();

            Serial.print("Distance: ");
            Serial.println(distance);

            delay(1000);

            if (distance > maxDistance)
            {
                maxDistance = distance;
            }
        }

        return maxDistance;
    }
};