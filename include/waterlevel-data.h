#pragma once

#include <Arduino.h>

struct __attribute__((packed)) WaterLevelData
{
    int level;
    double distance;
    bool isPumpOn;
    long fullThreshold;
    long emptyThreshold;
    bool alarmEnabled;
    char time[9];

    WaterLevelData(int l, double d, bool o, long eth, long fth, bool alarm = false, const char *t = "00:00:00")
    {
        level = l;
        distance = d;
        isPumpOn = o;
        fullThreshold = fth;
        emptyThreshold = eth;
        alarmEnabled = alarm;

        strncpy(time, t, 8);
        time[8] = '\0'; // Ensure null termination
    }

    WaterLevelData(double l, double d, bool o, long eth, long fth, bool alarm = false, const char *t = "00:00:00")
        : WaterLevelData(static_cast<int>(round(l)), d, o, eth, fth, alarm, t) {}

    char *formatForSSEvent()
    {
        int bufferSize = 100;

        char *buffer = new char[bufferSize];

        // Format the data into the allocated buffer
        snprintf(
            buffer,
            bufferSize,
            "{\"level\": %d, \"distance\": %.2f, \"isPumpOn\": %s, \"time\": \"%s\"}\n\n",
            level,
            distance,
            isPumpOn ? "true" : "false",
            time);

        return buffer;
    }
};
