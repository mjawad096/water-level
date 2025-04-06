#pragma once

#include <Arduino.h>

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
