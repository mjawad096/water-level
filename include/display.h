#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

#pragma once

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

class Display
{
private:
    Adafruit_SSD1306 display;
    bool dispalyInitialized;
    int level;

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

        level = -1;
    }

    void displayLevel(int level)
    {
        if (level == this->level)
        {
            return;
        }

        this->level = level;

        if (!dispalyInitialized)
        {
            return;
        }

        int levelStartCursor = 15;

        if (level == 100)
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
        display.setTextSize(5);
        display.print(level);
        display.println('%');

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

        display.setTextSize(1);
        display.println(text);

        display.display();
    }
};
