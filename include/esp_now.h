#include <ESP8266WiFi.h>
#include <espnow.h>

#pragma once

class EspNow
{
private:
    uint8_t broadcastAddress[6] = {0x08, 0xA6, 0xF7, 0xA8, 0x72, 0x1C};

public:
    void setup()
    {
        // Set device as a Wi-Fi Station
        WiFi.mode(WIFI_STA);

        // Init ESP-NOW
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
        esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
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