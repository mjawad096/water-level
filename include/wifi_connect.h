#include <ESP8266WiFi.h>

#pragma once

class WifiConnect
{
private:
    const String ssid = "WL_AB1C82AB1C82";

public:
    void setup()
    {
        WiFi.mode(WIFI_STA);

        connectWifi();
    }

    void connectWifi()
    {
        WiFi.begin(ssid);

        Serial.println("Connecting to wifi: " + ssid);

        // Wait for connection
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }

        Serial.println("Connected to WiFi");
    }
};