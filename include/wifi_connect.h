#include <ESP8266WiFi.h>
#include <display.h>

#pragma once

class WifiConnect
{
private:
    Display *display;

    const String ssid = "WL_AB1C82AB1C82";

public:
    void setup(Display *display)
    {
        this->display = display;

        WiFi.mode(WIFI_STA);

        connectWifi();
    }

    void connectWifi()
    {
        WiFi.begin(ssid);

        Serial.println("Connecting to wifi: " + ssid);

        display->displayText("Connecting to WiFi...", false);

        // Wait for connection
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }

        Serial.println("Connected to WiFi");

        display->displayText("Connected to WiFi", false);

        delay(2000);
    }
};