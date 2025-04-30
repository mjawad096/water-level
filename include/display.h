#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <waterlevel.h>
#include <telnet_logger.h>

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
    unsigned long levelDisplayStartMillis = -1;

    // Pixel refresher variables like a balck and white screen flashing
    bool refresherRunning = true;
    int refresherCycle = 0;
    unsigned long lastRefresherStep = 0;
    const int totalRefresherCycles = 100;
    const unsigned long refresherInterval = 400.0; // ms

    // Heading marquee variables
    String marqueeText1 = "";
    String marqueeText2 = "--------------------";
    int marqueeOffset = 0;
    unsigned long lastMarqueeUpdate = 0;
    const unsigned long marqueeSpeed = 100; // in ms

    // Time animation variables
    int timeOffsetX = 0;
    int timeOffsetDir = 1; // 1 = right, -1 = left
    unsigned long lastTimeOffsetUpdate = 0;
    const unsigned long timeOffsetInterval = 300; // ms
    const int timeOffsetMax = 20;                 // Max movement in pixels`

    // Time animation variables
    int levelOffsetX = 0;
    int levelOffsetDir = 1; // 1 = right, -1 = left
    unsigned long lastLevelOffsetUpdate = 0;
    const unsigned long levelOffsetInterval = 200; // ms
    int levelOffsetMax = 32;                       // Max movement in pixels`

    bool otaInProgress = false;

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
        // Night time is from 00 (12Am) to 3:59 AM (before 4)
        return (hour >= 00 && hour < 4);
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

    void dimDisplay(bool dim)
    {
        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command(dim ? 0x10 : 0xFF); // 0x10 = dimmed, 0xFF = full brightness
    }

    void updateTheAnimationOffsets()
    {
        unsigned long currentMillis = millis();

        display.invertDisplay(isNightTime());
        dimDisplay(isNightTime());

        if (millis() - lastMarqueeUpdate > marqueeSpeed)
        {
            marqueeOffset--;
            lastMarqueeUpdate = millis();

            if (-marqueeOffset > SCREEN_WIDTH) // 6 pixels per char approx
            {
                marqueeOffset = SCREEN_WIDTH; // reset off-screen
            }
        }

        if (millis() - lastTimeOffsetUpdate >= timeOffsetInterval)
        {
            timeOffsetX += timeOffsetDir;

            if (timeOffsetX >= timeOffsetMax || timeOffsetX <= 0)
            {
                timeOffsetDir *= -1; // Change direction
            }

            lastTimeOffsetUpdate = millis();
        }

        if (millis() - lastLevelOffsetUpdate >= levelOffsetInterval)
        {
            levelOffsetX += levelOffsetDir;

            if (levelOffsetX >= levelOffsetMax || levelOffsetX <= 0)
            {
                if (levelOffsetX >= levelOffsetMax)
                {
                    levelOffsetX = levelOffsetMax;
                }

                levelOffsetDir *= -1; // Change direction
            }

            lastLevelOffsetUpdate = millis();
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
            LOGL(F("SSD1306 allocation failed"));

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

    void setApSSID(String apSSID)
    {
        this->apSSID = apSSID;
    }

    void setOtaInProgress(bool otaInProgress)
    {
        this->otaInProgress = otaInProgress;
    }

    void displayLevel()
    {
        if (otaInProgress || !dispalyInitialized || levelData == nullptr)
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

        marqueeText1 = "Water Level (Wifi:" + String(WiFi.status() == WL_CONNECTED ? "V" : "X") + ")";

        levelOffsetMax = 80;

        if (levelData->level > 99)
        {
            levelOffsetMax = 32;
        }
        else if (levelData->level == -1 || levelData->level > 9)
        {
            levelOffsetMax = 56;
        }

        updateTheAnimationOffsets();

        display.clearDisplay();

        display.setTextColor(SSD1306_WHITE);

        display.setCursor(0, 0);

        if (levelData->isPumpOn)
        {
            display.setTextSize(2);
            display.print("Level:");
            display.print(levelData->level);
            display.println('%');

            display.setCursor(0, 25);
            display.setTextSize(3);
            display.println("PUMP:ON");
        }
        else
        {
            int numVisibleChars = marqueeText1.length();

            if (marqueeOffset > 2)
            {
                numVisibleChars = (int)((SCREEN_WIDTH - marqueeOffset) / 6); // Approx 6 pixels per char
            }

            display.setTextSize(1);
            display.setCursor(marqueeOffset, 0);
            display.print(marqueeText1.substring(0, numVisibleChars));
            display.setCursor(marqueeOffset, 8);
            display.print(marqueeText2.substring(0, numVisibleChars));

            display.setCursor(levelOffsetX, 20);
            display.setTextSize(4);
            display.print(levelData->level);
            display.println('%');
        }

        int infoCursorCol = 55;

        display.setTextSize(1);
        display.setCursor(0, infoCursorCol);

        if (!printIp())
        {
            display.setCursor(timeOffsetX, infoCursorCol);
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
            displayIp = (displayIp % 4) + 1;

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
            display.invertDisplay(false);
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
