#include <ESP8266WiFi.h>

#pragma once

class WifiConnect
{
private:
    const String ssid = "WL_1C72A8F7A608";

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