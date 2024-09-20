#include "WiFi.h"
#include "setting.h"
#include "display.h"

#pragma once

class WifiConnect
{
private:
    Setting *settings;
    Display *display;

public:
    WifiConnect()
    {
    }

    void setup(Setting *settings, Display *display)
    {
        this->settings = settings;
        this->display = display;

        WiFi.mode(WIFI_AP_STA);

        setupAccessPoint();

        connectWifi();
    }

    void setupAccessPoint()
    {
        String apSSID = getWifiAPName();

        WiFi.softAP(apSSID, "", 1, 1);

        IPAddress IP = WiFi.softAPIP();

        Serial.println("AP started:");
        Serial.print("SSID: ");
        Serial.println(apSSID);
        Serial.print("IP Address: ");
        Serial.println(IP);

        display->setApSSID(apSSID);
        display->displayText("AP started: " + apSSID, false);
    }

    String getWifiAPName()
    {
        uint64_t number = ESP.getEfuseMac();
        char hexString[17];
        snprintf(hexString, sizeof(hexString), "%04X%08X", (uint16_t)(number >> 32), (uint32_t)number);

        return String("WL_") + String(hexString);
    }

    void connectWifi()
    {
        Serial.print("Connecting to WiFi: ");
        Serial.println(settings->wifiSSID);

        display->displayText("Connecting to WiFi...", false);

        WiFi.begin(settings->wifiSSID, settings->wifiPassword);

        int maxAttempts = 30;

        for (int i = 0; i < maxAttempts; i++)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                break;
            }

            delay(1000);
            Serial.print(".");
        }

        Serial.println("");

        // Wait for connection
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Failed to connect to WiFi");
            return;
        }

        Serial.println("Connected to WiFi");

        display->displayText("Connected to WiFi", false);

        delay(2000);
    }
};