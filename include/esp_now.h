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
    static int retryCount;

    unsigned long lastSendAttemptMillis = 0;
    const unsigned long baseRetryInterval = 500; // Base interval 500ms
    const int maxRetries = 5;                    // Maximum retries before giving up

    static void resetRetry()
    {
        sentFailureForPeer2 = false;
        retryCount = 0;
    }

    unsigned long dynamicRetryInterval()
    {
        // Slightly increase retry interval for each failure (baseRetryInterval, 2*baseRetryInterval, etc.)
        return baseRetryInterval * (retryCount + 1);
    }

public:
    static WaterLevelData waterLevelData;
    static unsigned long lastUpdatedMillis;

    void setup(Display *display)
    {
        wifiConnect.setup(display, &waterLevelData);

        lastUpdatedMillis = millis();

        // Init ESP-NOW
        if (esp_now_init() != 0)
        {
            LOGL("Error initializing ESP-NOW");
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
            LOGL("Failed to add peer");
            return;
        }
    }

    void sendWaterLevelDataToPeer2()
    {
        if (!sentFailureForPeer2 || (millis() - lastSendAttemptMillis < dynamicRetryInterval()))
        {
            return;
        }

        if (retryCount < maxRetries)
        {
            lastSendAttemptMillis = millis();
            retryCount++;

            LOGF("Retrying... Attempt #%d, Retry Interval: %lu ms", retryCount, dynamicRetryInterval());

            int result = esp_now_send(broadcastAddress, (uint8_t *)&waterLevelData, sizeof(waterLevelData));

            if (result == 0)
            {
                LOGL("Data sent successfully (queued)");
            }
            else
            {
                LOG("Error sending data: ");
                LOGL(String(result));
            }
        }
        else
        {
            LOGL("Max retries reached. Giving up sending.");
            resetRetry();
        }
    }

    // Callback function that will be executed when data is received
    static void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
    {
        LOGF("Bytes received: %d", len);

        if (len < sizeof(WaterLevelData))
        {
            char message[32] = {0}; // make sure it's zero-initialized
            memcpy(message, incomingData, len);

            if (strcmp(message, "P2FAIL") == 0)
            {
                sentFailureForPeer2 = true;
                LOGL("P2FAIL received: Will resend water level data");
            }

            return;
        }

        memcpy(&waterLevelData, incomingData, sizeof(waterLevelData));

        lastUpdatedMillis = millis();

        LOGF("Water Level: %d, Distance: %.2f, Pump Status: %s, Full Threshold: %ld, Empty Threshold: %ld, Alarm Enabled: %s",
             waterLevelData.level,
             waterLevelData.distance,
             waterLevelData.isPumpOn ? "ON" : "OFF",
             waterLevelData.fullThreshold,
             waterLevelData.emptyThreshold,
             waterLevelData.alarmEnabled ? "ON" : "OFF");
    }

    // Callback when data is sent
    static void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
    {
        LOGL("Last Packet Send Status: ");

        LOG(sendStatus == 0 ? "Delivery success" : "Delivery fail");

        sentFailureForPeer2 = sendStatus != 0;

        if (sendStatus == 0)
        {
            // If delivery succeeded, reset retry counter
            retryCount = 0;
        }
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
int EspNow::retryCount = 0;