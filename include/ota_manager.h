#pragma once

#include <ArduinoOTA.h>
#include <WiFi.h>
#include <logger.h>

class OtaManager
{
private:
    static bool initialized;

    // Private constructor to prevent instantiation
    OtaManager() {}

    static void onStart()
    {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";

        LOGL("OTA Start: Updating " + type);
    }

    static void onEnd()
    {
        LOGL("OTA Update finished. Rebooting...");
    }

    static void onProgress(unsigned int progress, unsigned int total)
    {
        // Avoid heavy logging here, but optional light progress
        Serial.printf("OTA Progress: %u%%\r", (progress * 100) / total);
    }

    static void onError(ota_error_t error)
    {
        String message = "OTA Error: ";

        switch (error)
        {
        case OTA_AUTH_ERROR:
            message += "Auth Failed";
            break;
        case OTA_BEGIN_ERROR:
            message += "Begin Failed";
            break;
        case OTA_CONNECT_ERROR:
            message += "Connect Failed";
            break;
        case OTA_RECEIVE_ERROR:
            message += "Receive Failed";
            break;
        case OTA_END_ERROR:
            message += "End Failed";
            break;
        default:
            message += "Unknown";
            break;
        }

        LOGL(message);
    }

public:
    static void setup(const char *hostname = "ESP32-Waterlevel")
    {
        if (initialized)
            return;

        if (WiFi.status() != WL_CONNECTED)
        {
            LOGL("OTA setup failed: WiFi not connected.");
            return;
        }

        ArduinoOTA.setHostname(hostname);

        ArduinoOTA.onStart(onStart)
            .onEnd(onEnd)
            .onProgress(onProgress)
            .onError(onError);

        ArduinoOTA.begin();

        LOGL("OTA Ready. Hostname: " + String(hostname));

        initialized = true;
    }

    static void handle()
    {
        if (!initialized)
        {
            return;
        }

        ArduinoOTA.handle();
    }
};

// Define the static member
bool OtaManager::initialized = false;
