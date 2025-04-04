#pragma once

#include <WiFi.h>
#include <time.h>

class TimeManager
{
private:
    static bool initialized;
    static struct tm timeInfo;
    static unsigned long lastTimeUpdate;
    static const unsigned long timeUpdateInterval = 10000; // 10 seconds

public:
    static void setup()
    {
        if (initialized)
        {
            return;
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Time initialization failed due WIFI not connected.");
            return;
        }

        Serial.println("Initializing time...");

        int timeZoneOffset = 5; // UTC+5
        const char *ntpServer = "pool.ntp.org";

        configTime(timeZoneOffset * 3600, 0, ntpServer);

        Serial.println("Waiting for NTP time sync...");

        int attempts = 0;
        while (!getLocalTime(&timeInfo) && attempts < 10)
        {
            delay(1000);
            attempts++;
        }

        if (attempts < 10)
        {
            Serial.println("NTP Sync successful!");
            initialized = true;
        }
        else
        {
            Serial.println("Failed to get NTP time!");
        }
    }

    static void updateTime()
    {
        if (!initialized)
        {
            return;
        }

        unsigned long currentMillis = millis();

        // Only update the time if the debounce interval has passed
        if (currentMillis - lastTimeUpdate >= timeUpdateInterval)
        {
            if (getLocalTime(&timeInfo))
            {
                lastTimeUpdate = currentMillis;
            }
            else
            {
                Serial.println("Failed to update time!");
            }
        }
    }

    static String getFormattedTime()
    {
        if (!initialized)
        {
            Serial.println("Time not initialized.");
            return String(millis());
        }

        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
        return String(buffer);
    }
};

bool TimeManager::initialized = false;
struct tm TimeManager::timeInfo = {0};
unsigned long TimeManager::lastTimeUpdate = 0;
