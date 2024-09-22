#include "WiFi.h"
#include "setting.h"
#include "display.h"

#pragma once

class WifiConnect
{
private:
    Setting *settings;
    Display *display;

    unsigned long previousMillis = 0;
    const unsigned long interval = 300000;

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
        display->displayText("AP: " + apSSID, false);
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
        if (WiFi.status() == WL_CONNECTED)
        {
            return;
        }

        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= interval)
        {
            display->displayText("Wifi connection lost. Reconnecting...");

            previousMillis = currentMillis;

            connectWifi();
        }
    }

    void connectWifi()
    {
        Serial.print("Connecting to WiFi: ");
        Serial.println(settings->wifiSSID);

        display->displayText("Connecting to WiFi...", false);

        WiFi.begin(settings->wifiSSID, settings->wifiPassword);

        int maxAttempts = 30;
        int attempt = 0;
        while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts)
        {
            delay(500);
            Serial.print(".");
            attempt++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Connected to WiFi");
        }
        else
        {
            Serial.println("Failed to connect to WiFi");
        }

        display->displayText("Connected to WiFi", false);

        delay(2000);
    }
};