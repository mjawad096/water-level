#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <waterlevel.h>
#include <WiFi.h>

#pragma once

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

class Display
{
private:
    Adafruit_SSD1306 display;
    bool dispalyInitialized;
    String apSSID;
    unsigned long lastDisplayIpTime = 0;
    unsigned int displayIp = 1;

public:
    Display() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), dispalyInitialized(false)
    {
    }

    void setup()
    {
        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        {
            // Address 0x3D for 128x64
            Serial.println(F("SSD1306 allocation failed"));

            dispalyInitialized = false;
        }
        else
        {
            dispalyInitialized = true;
        }

        if (dispalyInitialized)
        {
            // Clear the buffer
            display.clearDisplay();
            display.display();
        }
    }

    void setApSSID(String apSSID)
    {
        this->apSSID = apSSID;
    }

    void displayLevel(WaterLevelData *levelData)
    {
        if (levelData == nullptr)
        {
            Serial.println("Error: Null water level data received.");
            return;
        }

        if (!dispalyInitialized)
        {
            Serial.println("Error: Display not initialized.");
            return;
        }

        int levelStartCursor = 15;

        if (levelData->level == 100)
        {
            levelStartCursor = 5;
        }

        display.clearDisplay();

        display.setTextColor(SSD1306_WHITE);

        display.setCursor(0, 0);
        display.setTextSize(1);
        display.print("Water Level");
        display.println(" (Wifi:" + String(WiFi.status() == WL_CONNECTED ? "V" : "X") + ")");
        display.println("-------------------");

        display.setCursor(levelStartCursor, 20);
        display.setTextSize(4);
        display.print(levelData->level);
        display.println('%');

        display.setCursor(0, 55);
        display.setTextSize(1);

        if (millis() - lastDisplayIpTime > 5000)
        {
            if (displayIp == 4)
            {
                displayIp = 1;
            }
            else
            {
                displayIp++;
            }

            lastDisplayIpTime = millis();
        }

        String ipMessage = "";
        String mac;

        switch (displayIp)
        {
        case 1:
            ipMessage = "IP: " + WiFi.localIP().toString();
            break;

        case 2:
            ipMessage = "SSID: " + apSSID;
            break;

        case 3:
            mac = WiFi.macAddress();
            mac.replace(":", "");
            ipMessage = "MAC: " + mac;
            break;

        case 4:
            mac = WiFi.softAPmacAddress();
            mac.replace(":", "");
            ipMessage = "APMAC: " + mac;
            break;
        }

        display.println(ipMessage);

        display.display();
    }

    void displayText(String text, bool clear = true)
    {
        if (!dispalyInitialized)
        {
            return;
        }

        if (clear)
        {
            display.clearDisplay();
            display.setCursor(0, 0);
        }

        display.setTextColor(SSD1306_WHITE);

        display.setTextSize(1);
        display.println(text);
        display.println("");

        display.display();
    }
};