#include <ArduinoJson.h>
#include "Setting.h"

#pragma once

class WaterLevel
{
private:
    const int echoPin = 18;
    const int trigPin = 19;
    const double speedOfSound = 0.034;

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

        return duration * speedOfSound / 2;
    }

    double getBestDistance()
    {
        double distances[3];
        double maxDistance = 0;

        for (int i = 0; i < 3; i++)
        {
            distances[i] = readDistance();
            Serial.print("Distance: ");
            Serial.println(distances[i]);
            delay(1000);

            if (distances[i] > maxDistance)
            {
                maxDistance = distances[i];
            }
        }

        return maxDistance;
    }

    int getLevel()
    {
        double deviceToWaterDistance = getBestDistance();

        Setting settings;

        double topEndDistanceFromDevice = settings.topEndFromDevice;
        double bottomEndDistanceFromDevice = settings.bottomEndFromDevice;

        double totalActualTankDepth = bottomEndDistanceFromDevice - topEndDistanceFromDevice;

        double currentEmptyDepth = deviceToWaterDistance - topEndDistanceFromDevice;

        double currentFilledDepth = totalActualTankDepth - currentEmptyDepth;

        double level = (currentFilledDepth / totalActualTankDepth) * 100;

        if (level < 0)
        {
            level = 0;
        }
        else if (level > 100)
        {
            level = 100;
        }

        return (int)round(level);
    }
};