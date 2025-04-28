#pragma once

#include <ESP8266WiFi.h>
#include <display.h>
#include <ota_manager.h>
#include <telnet_logger.h>

class WifiConnect
{
private:
    Display *display;

    const String ssid = "WL_1C72A8F7A608";

public:
    void setup(Display *display, WaterLevelData *waterLevelData)
    {
        this->display = display;

        WiFi.mode(WIFI_AP_STA);

        setupAccessPoint();

        connectWifi();

        TelnetLogger::setup(waterLevelData);
        OtaManager::setup(display);

        logInfo();

        delay(2000);
    }

    void logInfo()
    {
        IPAddress IP = WiFi.softAPIP();
        String apSSID = getWifiAPName();

        LOGF("AP started -> SSID: %s, IP Address: %s", apSSID.c_str(), IP.toString().c_str());

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
        // D1: WLD_84CCA881BD04 Kitchen
        // D2: WLD_807D3A4E8D08 Baramda
        // D3: WLD_ECFABC965FAA Washroom

        String mac = WiFi.macAddress();
        mac.replace(":", "");

        return "WLD_" + mac;
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