#include <ESP8266WiFi.h>
#include <display.h>
#include <ota_manager.h>

#pragma once

class WifiConnect
{
private:
    Display *display;

    const String ssid = "WL_1C72A8F7A608";

public:
    void setup(Display *display)
    {
        this->display = display;

        WiFi.mode(WIFI_STA);

        connectWifi();

        OtaManager::setup(display);

        delay(2000);
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
    }
};