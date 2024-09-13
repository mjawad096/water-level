#include <esp_now.h>
#include <WiFi.h>
#include <wifi_connect.h>
#include "waterlevel_data.h"

#pragma once

class EspNow
{
private:
    uint8_t broadcastAddress1[6] = {0x84, 0xCC, 0xA8, 0x81, 0xBD, 0x04};
    // uint8_t broadcastAddress2[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    esp_now_peer_info_t peerInfo;
    WifiConnect wifiConnect;

public:
    static WaterLevelData *waterLevelData;

    void setup(Setting *settings)
    {
        wifiConnect.setup(settings);

        EspNow::waterLevelData = new WaterLevelData();

        if (esp_now_init() != ESP_OK)
        {
            Serial.println("Error initializing ESP-NOW");
            return;
        }

        esp_now_register_send_cb(OnDataSent);

        // register peer
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        // register first peer
        memcpy(peerInfo.peer_addr, broadcastAddress1, 6);

        if (esp_now_add_peer(&peerInfo) != ESP_OK)
        {
            Serial.println("Failed to add peer");
            return;
        }

        //   // register second peer
        //   memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
        //   if (esp_now_add_peer(&peerInfo) != ESP_OK){
        //     Serial.println("Failed to add peer");
        //     return;
        //   }

        esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
    }

    void sendWaterLevel()
    {
        esp_err_t result = esp_now_send(0, (uint8_t *)&EspNow::waterLevelData->level, sizeof(EspNow::waterLevelData->level));

        if (result == ESP_OK)
        {
            Serial.println("Sent with success");
        }
        else
        {
            Serial.println("Error sending the data");
        }
    }

    // callback when data is sent
    static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
        char macStr[18];

        Serial.print("Packet to: ");

        // Copies the sender mac address to a string
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

        Serial.print(macStr);
        Serial.print(" Send status:\t");
        Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    }

    // callback when data is received
    static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
    {
        try
        {
            memcpy(EspNow::waterLevelData, incomingData, sizeof(WaterLevelData));
        }
        catch (const std::exception &e)
        {
            Serial.println("Error parsing data");
            Serial.print("Bytes received: ");
            Serial.println(len);
        }
    }
};

WaterLevelData *EspNow::waterLevelData = nullptr;