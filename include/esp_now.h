#pragma once

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <wifi_connect.h>
#include <display.h>
#include <waterlevel.h>

class EspNow
{
private:
    WifiConnect wifiConnect;
    Display *display;

    static uint8_t broadcastAddress[6]; // Baramda

    static bool sentFailureForPeer2; // Baramda

public:
    static WaterLevelData waterLevelData;
    static unsigned long lastUpdatedMillis;

    void setup(Display *display)
    {
        wifiConnect.setup(display);

        lastUpdatedMillis = millis();

        // Init ESP-NOW
        if (esp_now_init() != 0)
        {
            Serial.println("Error initializing ESP-NOW");
            return;
        }

        // Once ESPNow is successfully Init, we will register for recv CB to
        // Set device role to both Sender and Receiver
        esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

        // Register callbacks
        esp_now_register_recv_cb(OnDataRecv);
        esp_now_register_send_cb(OnDataSent);

        // Register peer
        if (esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0) != 0)
        {
            Serial.println("Failed to add peer");
            return;
        }
    }

    void sendWaterLevelDataToPeer2()
    {
        if (!sentFailureForPeer2)
        {
            return;
        }

        int result = esp_now_send(broadcastAddress, (uint8_t *)&waterLevelData, sizeof(waterLevelData));

        if (result == 0)
        {
            Serial.println("Data sent successfully");
        }
        else
        {
            Serial.print("Error sending data: ");
            Serial.println(result);
        }
    }

    // Callback function that will be executed when data is received
    static void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
    {
        Serial.printf("Bytes received: %d\n", len);

        if (len < sizeof(WaterLevelData))
        {
            char message[32] = {0}; // make sure it's zero-initialized
            memcpy(message, incomingData, len);

            if (strcmp(message, "P2FAIL") == 0)
            {
                sentFailureForPeer2 = true;
                Serial.println("P2FAIL received: Will resend water level data");
            }

            return;
        }

        memcpy(&waterLevelData, incomingData, sizeof(waterLevelData));

        lastUpdatedMillis = millis();

        Serial.printf("Water Level: %d\nDistance: %.2f\nPump Status: %s\nFull Threshold: %ld\nEmpty Threshold: %ld\nAlarm Enabled: %s\nTime: %s\n",
                      waterLevelData.level,
                      waterLevelData.distance,
                      waterLevelData.isPumpOn ? "ON" : "OFF",
                      waterLevelData.fullThreshold,
                      waterLevelData.emptyThreshold,
                      waterLevelData.alarmEnabled ? "ON" : "OFF",
                      waterLevelData.time);

        Serial.printf("Size: %d\n", sizeof(waterLevelData));
    }

    // Callback when data is sent
    static void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
    {
        Serial.print("Last Packet Send Status: ");

        Serial.println(sendStatus == 0 ? "Delivery success" : "Delivery fail");

        sentFailureForPeer2 = sendStatus != 0;
    }

    bool isLastUpdatedMoreThan(int minutes)
    {
        return (millis() - lastUpdatedMillis) > (unsigned long)(minutes * 60 * 1000);
    }
};

uint8_t EspNow::broadcastAddress[6] = {0x80, 0x7D, 0x3A, 0x4E, 0x8D, 0x08}; // Baramda
WaterLevelData EspNow::waterLevelData = {-1, -1, false, -1, -1, false, "00:00:00 AM"};
unsigned long EspNow::lastUpdatedMillis = 0;
bool EspNow::sentFailureForPeer2 = false;