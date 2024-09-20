#include <ESP8266WiFi.h>
#include <espnow.h>
#include <wifi_connect.h>

#pragma once

class EspNow
{

private:
    WifiConnect wifiConnect;
    uint8_t broadcastAddress[6] = {0x08, 0xA6, 0xF7, 0xA8, 0x72, 0x1C};
    uint8_t broadcastAddress1[6] = {0x08, 0xA6, 0xF7, 0xA8, 0x72, 0x1D};

public:
    void setup()
    {
        wifiConnect.setup();

        if (esp_now_init() != 0)
        {
            Serial.println("Error initializing ESP-NOW");
            return;
        }

        // Once ESPNow is successfully Init, we will register for Send CB to
        // get the status of Trasnmitted packet
        esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
        esp_now_register_send_cb(OnDataSent);

        // Register peer
        if (esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 0, NULL, 0) != 0)
        {
            Serial.println("Failed to add peer");
            return;
        }

        if (esp_now_add_peer(broadcastAddress1, ESP_NOW_ROLE_SLAVE, 0, NULL, 0) != 0)
        {
            Serial.println("Failed to add peer");
            return;
        }
    }

    // Callback when data is sent
    static void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
    {
        Serial.print("Last Packet Send Status: ");

        if (sendStatus == 0)
        {
            Serial.println("Delivery success");
        }
        else
        {
            Serial.println("Delivery fail");
        }
    }

    void sendDistance(double distance)
    {
        esp_now_send(NULL, (uint8_t *)&distance, sizeof(distance));
    }
};