#include "Arduino.h"

#pragma once

struct WaterLevelData
{
    int level;
    double distance;

    WaterLevelData() : level(0), distance(0) {}

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