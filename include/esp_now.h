#include <ESP8266WiFi.h>
#include <espnow.h>
#include <display.h>

#pragma once

class EspNow
{
private:
    static Display display;

public:
    static int waterLevel;
    static long lastUpdatedMillis;

    void setup()
    {
        display.setup();

        WiFi.mode(WIFI_STA);
        lastUpdatedMillis = millis();

        // Init ESP-NOW
        if (esp_now_init() != 0)
        {
            Serial.println("Error initializing ESP-NOW");
            return;
        }

        // Once ESPNow is successfully Init, we will register for recv CB to
        // get recv packer info
        esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
        esp_now_register_recv_cb(OnDataRecv);
    }

    // Callback function that will be executed when data is received
    static void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
    {
        memcpy(&waterLevel, incomingData, sizeof(waterLevel));

        display.displayLevel(waterLevel);

        Serial.print("Bytes received: ");
        Serial.println(len);

        lastUpdatedMillis = millis();
    }

    bool isLastUpdatedMoreThan(int minutes)
    {
        return (millis() - lastUpdatedMillis) > (minutes * 60 * 1000);
    }
};

Display EspNow::display;
int EspNow::waterLevel = 0;
long EspNow::lastUpdatedMillis = 0;