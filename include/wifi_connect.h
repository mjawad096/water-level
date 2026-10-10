#pragma once

#include "WiFi.h"
#include "setting.h"
#include "display.h"
#include "led.h"
#include <logger.h>
#include <ota_manager.h>
#include <ESPmDNS.h>
#include <esp_netif.h>
#include <esp_netif_net_stack.h>
#include <lwip/etharp.h>

class WifiConnect
{
private:
    Setting *settings;
    Display *display;
    Led *led;
    Database *database;

    unsigned long previousMillis = 0;
    const unsigned long interval = 300000;

    static bool wifiConnecting;

    unsigned long lastArp = 0;
    static const unsigned long arpInterval = 30000;

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

        database->setup(&this->settings->debugLogging);

        LOG("\n\n");
        LOGL(" --- Database initialized --- ");

        display->displayText("DB: initialized");

        TelnetLogger::setup();
        OtaManager::setup(display);

        initMDNS();

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
        // WL_1C72A8F7A608
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
            wifiConnecting = false;

            previousMillis = currentMillis;

            announceOurselves();

            if (WiFi.status() == WL_CONNECTED && !TimeManager::isInitialized())
            {
                // If WiFi is connected but time is not initialized, restart the ESP
                ESP.restart();
            }

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

    // This board says nothing for minutes at a time, so the router drops it
    // from the table it answers ARP from, and then has neither an answer nor
    // a reason to pass the question on. A laptop that has just joined has to
    // ask, and waits -- 47 seconds, measured on the rf-switch board behind
    // this same router. A phone is never forgotten because a phone never
    // stops talking. So talk.
    //
    // The esp8266 core has a helper for this; esp32 does not, but it is the
    // same lwip underneath.
    void announceOurselves()
    {
        if (millis() - lastArp < arpInterval)
            return;

        lastArp = millis();

        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

        if (!netif)
            return;

        struct netif *stack = (struct netif *)esp_netif_get_netif_impl(netif);

        if (stack)
            etharp_gratuitous(stack);
    }

    void connectWifi()
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            wifiConnecting = false;

            return;
        }

        wifiConnecting = true;

        LOGF("Connecting to WiFi: %s", settings->wifiSSID);

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

        if (WiFi.status() == WL_CONNECTED)
        {
            LOGL("Connected to WiFi.");
            display->displayText("Connected to WiFi", false);
        }
        else
        {
            LOGL("Failed to connect to WiFi.");
            display->displayText("Failed to connect to WiFi", false);
        }

        wifiConnecting = false;

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

    static bool isWifiConnecting()
    {
        return wifiConnecting;
    }
};

bool WifiConnect::wifiConnecting = false;
