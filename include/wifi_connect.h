#pragma once

#include "WiFi.h"
#include "setting.h"
#include "display.h"
#include "led.h"
#include <logger.h>
#include <ota_manager.h>
#include <ESPmDNS.h>

class WifiConnect
{
private:
    Setting *settings;
    Display *display;
    Led *led;
    Database *database;

    unsigned long previousMillis = 0;
    const unsigned long interval = 300000;

public:
    WifiConnect()
    {
    }

    void setup(Setting *settings, Display *display, Led *led, Database *database)
    {
        this->settings = settings;
        this->display = display;
        this->led = led;
        this->database = database;

        WiFi.mode(WIFI_AP_STA);

        setupAccessPoint();

        connectWifi();

        TimeManager::setup();

        database->setup();

        LOGL("\n\n --- Database initialized --- ");

        TelnetLogger::setup();
        OtaManager::setup();

        initMDNS();

        logInfo();
    }

    void logInfo()
    {
        IPAddress IP = WiFi.softAPIP();
        String apSSID = getWifiAPName();

        LOGF("AP started -> SSID: %s, IP Address: %s\n", apSSID.c_str(), IP.toString().c_str());

        display->setApSSID(apSSID);
        display->displayText("AP: " + apSSID, false);
    }

    void setupAccessPoint()
    {
        String apSSID = getWifiAPName();

        WiFi.softAP(apSSID, "", 1, 1);
    }

    String getWifiAPName()
    {
        uint64_t number = ESP.getEfuseMac();
        char hexString[17];
        snprintf(hexString, sizeof(hexString), "%04X%08X", (uint16_t)(number >> 32), (uint32_t)number);

        return String("WL_") + String(hexString);
    }

    void checkWifiConnection()
    {
        unsigned long currentMillis = millis();

        if (WiFi.status() == WL_CONNECTED)
        {
            previousMillis = currentMillis;

            return;
        }

        if (currentMillis - previousMillis >= interval)
        {
            display->displayText("Wifi connection lost. Reconnecting...");
            LOGL("Wifi connection lost. Reconnecting...");

            previousMillis = currentMillis;

            connectWifi();
        }
    }

    void connectWifi()
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            return;
        }

        LOGL("Connecting to WiFi: " + settings->wifiSSID);

        display->displayText("Connecting to WiFi...", false);

        led->on();

        WiFi.begin(settings->wifiSSID, settings->wifiPassword);

        int maxAttempts = 30;
        int attempt = 0;
        while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts)
        {
            delay(500);
            LOG(".");
            attempt++;
        }

        LOG("\n");

        if (WiFi.status() == WL_CONNECTED)
        {
            LOGL("Connected to WiFi.");
        }
        else
        {
            LOGL("Failed to connect to WiFi.");
        }

        display->displayText("Connected to WiFi", false);

        delay(2000);

        led->off();
    }

    void initMDNS()
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            LOGL("Connected to WiFi.");

            // Start mDNS with a hostname
            if (MDNS.begin("esp32-waterlevel")) // This makes your ESP32 accessible as esp32-waterlevel.local
            {
                LOGL("mDNS responder started");
            }
            else
            {
                LOGL("Error setting up mDNS responder");
            }
        }
        else
        {
            LOGL("Failed to connect to WiFi.");
        }
    }
};