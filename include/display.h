#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <waterlevel.h>

#pragma once

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

class Display
{
private:
    Adafruit_SSD1306 display;
    bool dispalyInitialized;
    unsigned long lastDisplayIpTime = 0;
    unsigned int displayIp = 1;
    unsigned long levelDisplayStartMillis = -1;

    int xOffset = 0;
    int yOffset = 0;
    unsigned long lastShiftTime = 0;
    unsigned long lastInvertTime = 0;

    bool refresherRunning = true;
    int refresherCycle = 0;
    unsigned long lastRefresherStep = 0;
    const int totalRefresherCycles = 100;
    const unsigned long refresherInterval = 400.0; // ms

    WaterLevelData *levelData = nullptr;

    bool isNightTime(bool checkForMid = false)
    {
        if (levelData == nullptr)
            return false;

        String timeStr = levelData->time;
        if (timeStr.length() < 10)
            return false; // Basic sanity check

        // Extract hours, minutes, and AM/PM
        int hour = timeStr.substring(0, 2).toInt();
        int minute = timeStr.substring(3, 5).toInt();
        String ampm = timeStr.substring(9, 11); // "AM" or "PM"

        // Convert to 24-hour format
        if (ampm == "PM" && hour != 12)
        {
            hour += 12;
        }
        else if (ampm == "AM" && hour == 12)
        {
            hour = 0;
        }

        if (checkForMid)
        {
            return (hour == 0 && minute == 0);
        }

        // Now hour is in 0–23
        // Night time is from 18 (6PM) to 5:59 AM (before 6)
        return (hour >= 18 || hour < 6);
    }

    bool isMidnight()
    {
        return isNightTime(true);
    }

    void handlePixelRefresher()
    {
        unsigned long currentMillis = millis();

        if (isMidnight() && !refresherRunning)
        {
            refresherRunning = true;
            refresherCycle = 0;
            lastRefresherStep = currentMillis;
        }

        // Handle ongoing refresher cycles
        if (refresherRunning && (currentMillis - lastRefresherStep) >= refresherInterval)
        {
            if (refresherCycle >= totalRefresherCycles)
            {
                refresherRunning = false;
                display.clearDisplay(); // Clear after last step
                display.display();
            }
            else
            {
                display.clearDisplay();
                uint16_t color = (refresherCycle % 2 == 0) ? SSD1306_WHITE : SSD1306_BLACK;
                display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
                display.display();

                refresherCycle++;
                lastRefresherStep = currentMillis;
            }
        }
    }

    void updateDisplayEffects()
    {
        unsigned long currentMillis = millis();

        display.invertDisplay(isNightTime());

        // Pixel shift every 10 seconds
        if (currentMillis - lastShiftTime >= 10000)
        {
            // Cycle shift in range [-1, 1]
            xOffset = (xOffset + 1) % 3 - 1;
            yOffset = (yOffset + 1) % 3 - 1;
            lastShiftTime = currentMillis;
        }
    }

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

    void setLevelData(WaterLevelData *data)
    {
        levelData = data;
    }

    void displayLevel()
    {
        if (!dispalyInitialized || levelData == nullptr)
        {
            return;
        }

        if (levelDisplayStartMillis == -1)
        {
            levelDisplayStartMillis = millis();
        }

        handlePixelRefresher();

        if (refresherRunning)
        {
            return;
        }

        updateDisplayEffects();

        display.clearDisplay();

        display.setTextColor(SSD1306_WHITE);

        display.setCursor(xOffset, yOffset);

        if (levelData->isPumpOn)
        {
            display.setTextSize(2);
            display.print("Level:");
            display.print(levelData->level);
            display.println('%');

            display.setCursor(xOffset, 25 + yOffset);
            display.setTextSize(3);
            display.println("PUMP:ON");
        }
        else
        {
            display.setTextSize(1);
            display.print("Water Level");
            display.println(" (Wifi:" + String(WiFi.status() == WL_CONNECTED ? "V" : "X") + ")");
            display.println("--------------------");

            int levelStartCursor = 40;

            if (levelData->level > 99)
            {
                levelStartCursor = 16;
            }
            else if (levelData->level == -1 || levelData->level > 9)
            {
                levelStartCursor = 28;
            }

            display.setCursor(levelStartCursor + xOffset, 20 + yOffset);
            display.setTextSize(4);
            display.print(levelData->level);
            display.println('%');
        }

        int infoCursorCol = 55;

        display.setTextSize(1);
        display.setCursor(xOffset, infoCursorCol + yOffset);

        if (!printIp())
        {
            display.setCursor(10 + xOffset, infoCursorCol + yOffset);
            display.println("Time: " + String(levelData->time));
        }

        display.display();
    }

    bool printIp()
    {
        if (!canPrintIp())
        {
            return false;
        }

        if (millis() - lastDisplayIpTime > 5000)
        {
            if (displayIp == 3)
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
            mac = WiFi.macAddress();
            mac.replace(":", "");
            ipMessage = "MAC: " + mac;
            break;

        case 3:
            mac = WiFi.softAPmacAddress();
            mac.replace(":", "");
            ipMessage = "APMAC: " + mac;
            break;
        }

        display.println(ipMessage);

        return true;
    }

    bool canPrintIp()
    {
        if (millis() - levelDisplayStartMillis > (60 * 1000))
        {
            return false;
        }

        return true;
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
