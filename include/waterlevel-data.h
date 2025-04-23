#pragma once

#include <Arduino.h>

#define WD_BUFFER_SIZE 100

struct __attribute__((packed)) WaterLevelData
{
    int level;
    double distance;
    bool isPumpOn;
    long fullThreshold;
    long emptyThreshold;
    bool alarmEnabled;
    char time[12]; // HH:MM:SS AM + null terminator

    WaterLevelData(int l, double d, bool o, long eth, long fth, bool alarm = false, const char *t = "00:00:00")
    {
        level = l;
        distance = d;
        isPumpOn = o;
        fullThreshold = fth;
        emptyThreshold = eth;
        alarmEnabled = alarm;

        int maxCopyLen = sizeof(time) - 1;
        strncpy(time, t, maxCopyLen);
        time[maxCopyLen] = '\0'; // Ensure null termination
    }

    WaterLevelData(double l, double d, bool o, long eth, long fth, bool alarm = false, const char *t = "00:00:00")
        : WaterLevelData(static_cast<int>(round(l)), d, o, eth, fth, alarm, t) {}

    void formatForSSEvent(char *buffer)
    {
        // Format the data into the allocated buffer
        snprintf(
            buffer,
            WD_BUFFER_SIZE,
            "{\"level\": %d, \"distance\": %.2f, \"isPumpOn\": %s, \"time\": \"%s\"}\n\n",
            level,
            distance,
            isPumpOn ? "true" : "false",
            time);
    }
};
