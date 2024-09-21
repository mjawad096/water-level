#include <ArduinoJson.h>
#include "Setting.h"

#pragma once

struct WaterLevelData
{
    int level;
    double distance;

    WaterLevelData(int l, double d) : level(l), distance(d) {}

    WaterLevelData(double l, double d) : level((int)round(l)), distance(d) {}

    char *formatForSSEvent()
    {
        int bufferSize = 50;

        char *buffer = new char[bufferSize];

        // Format the data into the allocated buffer
        snprintf(buffer, bufferSize, "{\"level\": %d, \"distance\": %.2f}\n\n", level, distance);

        return buffer;
    }
};

class WaterLevel
{
private:
    Setting *settings;

public:
    static double deviceToWaterDistance;
    static unsigned long lastUpdatedMillis;

    void setup(Setting *settings)
    {
        this->settings = settings;
    }

    WaterLevelData getLevel()
    {
        if (deviceToWaterDistance == -1)
        {
            return WaterLevelData(-1, deviceToWaterDistance);
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

        return WaterLevelData(level, WaterLevel::deviceToWaterDistance);
    }

    static bool isLastUpdatedMoreThan(unsigned long minutes);
};

double WaterLevel::deviceToWaterDistance = -1;
unsigned long WaterLevel::lastUpdatedMillis = 0;

bool WaterLevel::isLastUpdatedMoreThan(unsigned long minutes)
{
    return (millis() - lastUpdatedMillis) > (minutes * 60 * 1000);
}