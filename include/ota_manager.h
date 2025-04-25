#pragma once

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <display.h>

enum UpdateType
{
    NONE,
    FLASH,
    FILESYSTEM,
};

class OtaManager
{
private:
    static bool initialized;
    static unsigned int lastProgress;

    static UpdateType updateType;
    static Display *display;

    // Private constructor to prevent instantiation
    OtaManager() {}

    static void onStart()
    {
        updateType = (ArduinoOTA.getCommand() == U_FLASH) ? UpdateType::FLASH : UpdateType::FILESYSTEM;

        display->setOtaInProgress(true);

        display->displayText("OTA: Started");
    }

    static void onEnd()
    {
        display->displayText("OTA: Rebooting...", false);

        delay(1500);
    }

    static void onProgress(unsigned int progress, unsigned int total)
    {
        unsigned int currentProgress = static_cast<unsigned int>((progress * 100) / total);

        if (currentProgress == lastProgress || (currentProgress % 3 != 0 && currentProgress != 100))
            return;

        display->displayText("OTA: Started");

        display->displayText("OTA: " + String(updateType == UpdateType::FLASH ? "Sketch..." : "Filesystem..."), false);

        display->displayText("OTA: Progress " + String(currentProgress) + "%", false);

        lastProgress = currentProgress;
    }

    static void onError(ota_error_t error)
    {
        String message = "OTA: ";

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
            message += "Unknown Error";
            break;
        }

        display->displayText(message, false);
    }

public:
    static void setup(Display *d)
    {
        if (initialized)
            return;

        display = d;

        display->displayText("OTA: Initializing");

        if (WiFi.status() != WL_CONNECTED)
        {
            display->displayText("OTA: NO WIFI.", true);
            return;
        }

        String hostname = WiFi.macAddress();

        hostname.replace(":", "");

        ArduinoOTA.setHostname(hostname.c_str());

        ArduinoOTA.onStart(onStart);
        ArduinoOTA.onEnd(onEnd);
        ArduinoOTA.onProgress(onProgress);
        ArduinoOTA.onError(onError);

        ArduinoOTA.begin();

        display->displayText("OTA: Ready", false);

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
unsigned int OtaManager::lastProgress = 0;
UpdateType OtaManager::updateType = UpdateType::NONE;
Display *OtaManager::display = nullptr;
