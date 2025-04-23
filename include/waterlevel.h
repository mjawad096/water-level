#pragma once

struct __attribute__((packed)) WaterLevelData
{
    int level;
    double distance;
    bool isPumpOn;
    long fullThreshold;
    long emptyThreshold;
    bool alarmEnabled;
    char time[12];
};