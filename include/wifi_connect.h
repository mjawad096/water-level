#include "ArduinoJson.h"
#include "WiFi.h"
// #include "clock.h"
#include "setting.h"

#pragma once

class WifiConnect
{
private:
    int max_retries = 20;
    bool connected = false;
    Setting settings;

public:
    WifiConnect()
    {
    }

    void setup()
    {
        WiFi.mode(WIFI_AP_STA);

        settings.setup();

        setupAccessPoint();

        // connectWifi();
    }

    void connectWifi()
    {
        String wifiSsid = "";
        String wifiPassword = "";

        wifiSsid = settings.wifiSSID;
        wifiPassword = settings.wifiPassword;

        if (wifiSsid.length() < 5 || wifiPassword.length() < 5)
        {
            Serial.println("No WiFi settings found, Not connecting to WiFi.");
            return void();
        }

        Serial.print("Connecting to Wifi: ");
        Serial.println(wifiSsid);

        WiFi.begin(wifiSsid, wifiPassword);

        setTimeWhenConnected();
    }

    void setTimeWhenConnected()
    {
        int tryCount = 0;

        while (!isConnected() && tryCount < max_retries)
        {
            digitalWrite(BUILTIN_LED, HIGH);
            delay(200);
            digitalWrite(BUILTIN_LED, LOW);
            Serial.print(".");
            tryCount++;
        }

        Serial.println("");

        if (!isConnected())
        {
            Serial.println("Unable to connect to wifi.");
            return;
        }

        connected = true;
        digitalWrite(BUILTIN_LED, HIGH);

        Serial.print("Connected, IP address:");
        Serial.println(WiFi.localIP());
    }

    bool isConnected()
    {
        return connected = WiFi.status() == WL_CONNECTED;
    }

    void setupAccessPoint()
    {
        String wifiSsid = getWifiAPName();
        String apPassword = "12345678";

        Setting settings;

        if (settings.apSSID.length() > 4)
        {
            wifiSsid = getWifiAPName() + String("_") + settings.apSSID;
        }

        if (settings.apPassword.length() > 4)
        {
            apPassword = settings.apPassword;
        }

        WiFi.softAP(wifiSsid, apPassword);

        IPAddress IP = WiFi.softAPIP();

        Serial.println("Wifi AP started:");
        Serial.print("SSID: ");
        Serial.println(wifiSsid);
        Serial.print("IP Address: ");
        Serial.println(IP);
    }

    String getAvailableNetworks(bool startScan = false)
    {
        String response = "";
        JsonDocument doc;

        if (startScan)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                WiFi.disconnect();
            }

            WiFi.scanNetworks(true, true);
            doc["status"] = "scanning";
            serializeJson(doc, response);
            return response;
        }

        int n = WiFi.scanComplete();

        if (n == WIFI_SCAN_RUNNING)
        {
            log_i("scan running");
            doc["status"] = "scanning";
            serializeJson(doc, response);
            return response;
        }

        if (n == WIFI_SCAN_FAILED)
        {
            log_i("scan failed");
            doc["status"] = "scan_failed";
            serializeJson(doc, response);
            return response;
        }

        doc["status"] = "success";

        JsonArray networks = doc["networks"].to<JsonArray>();

        for (int i = 0; i < n; i++)
        {
            JsonObject network = networks.add<JsonObject>();
            network["ssid"] = WiFi.SSID(i);
            network["rssi"] = WiFi.RSSI(i);
            network["encryption"] = WiFi.encryptionType(i);
            network["channel"] = WiFi.channel(i);
            network["bssid"] = WiFi.BSSIDstr(i);
        }
        serializeJson(doc, response);

        log_i("Networks %s", response.c_str());

        WiFi.setAutoConnect(true);

        return response;
    }

    String getWifiAPName()
    {
        uint64_t number = ESP.getEfuseMac();
        char hexString[17];
        snprintf(hexString, sizeof(hexString), "%04X%08X", (uint16_t)(number >> 32), (uint32_t)number);
        return String("STAP_") + String(hexString);
    }
};