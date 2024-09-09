#include "WiFi.h"
#include "setting.h"

#pragma once

class WifiConnect
{
private:
    Setting *settings;

public:
    WifiConnect()
    {
    }

    void setup(Setting *settings)
    {
        this->settings = settings;

        WiFi.mode(WIFI_AP_STA);

        setupAccessPoint();
    }

    void setupAccessPoint()
    {
        String apSSID = getWifiAPName();
        String apPassword = "12345678";

        if (settings->apSSID.length() > 4)
        {
            apSSID = getWifiAPName() + String("_") + settings->apSSID;
        }

        if (settings->apPassword.length() > 4)
        {
            apPassword = settings->apPassword;
        }

        WiFi.softAP(apSSID, apPassword);

        IPAddress IP = WiFi.softAPIP();

        Serial.println("Wifi AP started:");
        Serial.print("SSID: ");
        Serial.println(apSSID);
        Serial.print("IP Address: ");
        Serial.println(IP);
    }

    String getWifiAPName()
    {
        uint64_t number = ESP.getEfuseMac();
        char hexString[17];
        snprintf(hexString, sizeof(hexString), "%04X%08X", (uint16_t)(number >> 32), (uint32_t)number);

        return String("WL_") + String(hexString);
    }
};