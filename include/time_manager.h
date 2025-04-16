#pragma once

#include <WiFi.h>
#include <time.h>

class TimeManager
{
private:
    static bool initialized;
    static struct tm timeInfo;
    static unsigned long lastTimeUpdate;
    static const unsigned long timeUpdateInterval = 1000; // 1 seconds

    // Private constructor to prevent instantiation
    TimeManager() {}

    static String getDateTimeString(struct tm *timeInfo)
    {
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
        return String(buffer);
    }

    static String getDateString(struct tm *timeInfo)
    {
        if (timeInfo == nullptr)
        {
            return String(millis());
        }

        char buffer[11];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeInfo);

        return String(buffer);
    }

    static String getTimeString(struct tm *timeInfo)
    {
        if (timeInfo == nullptr)
        {
            return String(millis());
        }

        char buffer[11];
        strftime(buffer, sizeof(buffer), "%H:%M:%S", timeInfo);

        return String(buffer);
    }

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

    static bool isInitialized()
    {
        return initialized;
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

    static String getDateTimeString()
    {
        if (!initialized)
        {
            Serial.println("Time not initialized.");
            return String(millis());
        }

        return getDateTimeString(&timeInfo);
    }

    static String getDateString()
    {
        if (!initialized)
        {
            Serial.println("Time not initialized.");
            return String(millis());
        }

        return getDateString(&timeInfo);
    }

    static String getTimeString()
    {
        if (!initialized)
        {
            Serial.println("Time not initialized.");
            return String(millis());
        }

        return getTimeString(&timeInfo);
    }

    static struct tm getTimeInfoDaysAgo(int daysAgo)
    {
        if (!initialized)
        {
            Serial.println("Time not initialized.");
            return {0};
        }

        struct tm timeInfoCopy = timeInfo; // Create a copy of the current time

        timeInfoCopy.tm_mday -= daysAgo; // Subtract the number of days
        mktime(&timeInfoCopy);           // Normalize time structure

        return timeInfoCopy;
    }

    static String getDateDaysAgoString(int daysAgo)
    {
        struct tm timeInfoCopy = getTimeInfoDaysAgo(daysAgo);

        return getDateString(&timeInfoCopy);
    }

    // Validate "YYYY-MM-DD"
    static bool isValidDate(const String &s)
    {
        if (s.length() != 10)
            return false;

        struct tm tm;
        memset(&tm, 0, sizeof(tm));

        // Try to parse; strptime returns a pointer to the first unparsed character
        char *ret = strptime(s.c_str(), "%Y-%m-%d", &tm);
        if (ret == nullptr || *ret != '\0')
        {
            // parsing failed or extra chars remain
            return false;
        }

        // mktime will normalize out‐of‐range values (e.g. month=13 → next year)
        // so if you get -1 back, it was an invalid date
        time_t t = mktime(&tm);
        return (t != (time_t)-1);
    }
};

bool TimeManager::initialized = false;
struct tm TimeManager::timeInfo = {0};
unsigned long TimeManager::lastTimeUpdate = 0;
