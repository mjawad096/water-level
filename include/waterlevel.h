#include <ArduinoJson.h>
#include "setting.h"
#include "current_sensor.h"

#pragma once

struct __attribute__((packed)) WaterLevelData
{
    int level;
    double distance;
    bool isPumpOn;
    long fullThreshold;
    long emptyThreshold;

    WaterLevelData(int l, double d, bool o, long eth, long fth) : level(l),
                                                                  distance(d),
                                                                  isPumpOn(o),
                                                                  emptyThreshold(eth),
                                                                  fullThreshold(fth)
    {
    }

    WaterLevelData(double l, double d, bool o, long eth, long fth) : level((int)round(l)),
                                                                     distance(d),
                                                                     isPumpOn(o),
                                                                     emptyThreshold(eth),
                                                                     fullThreshold(fth)
    {
    }

    char *formatForSSEvent()
    {
        int bufferSize = 70;

        char *buffer = new char[bufferSize];

        // Format the data into the allocated buffer
        snprintf(buffer, bufferSize, "{\"level\": %d, \"distance\": %.2f, \"isPumpOn\": %s}\n\n", level, distance, isPumpOn ? "true" : "false");

        return buffer;
    }
};

class WaterLevel
{
private:
    Setting *settings;
    CurrentSensor *currentSensor;

public:
    static double deviceToWaterDistance;
    static unsigned long lastUpdatedMillis;

    void setup(Setting *settings, CurrentSensor *currentSensor)
    {
        this->settings = settings;
        this->currentSensor = currentSensor;
    }

    WaterLevelData getLevel()
    {
        if (deviceToWaterDistance == -1)
        {
            return WaterLevelData(-1, deviceToWaterDistance, currentSensor->isCurrentFlowing(), settings->emptyThreshold, settings->fullThreshold);
        }

        double topEndDistanceFromDevice = settings->topEndFromDevice;
        double bottomEndDistanceFromDevice = settings->bottomEndFromDevice;

        double totalActualTankDepth = bottomEndDistanceFromDevice - topEndDistanceFromDevice;

        double currentEmptyDepth = WaterLevel::deviceToWaterDistance - topEndDistanceFromDevice;

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

        // Serial.print("Water Level: ");
        // Serial.println(level);
        // Serial.print("Distance: ");
        // Serial.println(deviceToWaterDistance);
        // Serial.print("Pump Status: ");
        // Serial.println(currentSensor->isCurrentFlowing() ? "ON" : "OFF");
        // Serial.print("Full Threshold: ");
        // Serial.println(settings->fullThreshold);
        // Serial.print("Empty Threshold: ");
        // Serial.println(settings->emptyThreshold);
        // Serial.println();

        return WaterLevelData(level, WaterLevel::deviceToWaterDistance, currentSensor->isCurrentFlowing(), settings->emptyThreshold, settings->fullThreshold);
    }

    static bool isLastUpdatedMoreThan(unsigned long minutes);
};

double WaterLevel::deviceToWaterDistance = -1;
unsigned long WaterLevel::lastUpdatedMillis = 0;

bool WaterLevel::isLastUpdatedMoreThan(unsigned long minutes)
{
    return (millis() - lastUpdatedMillis) > (minutes * 60 * 1000);
}