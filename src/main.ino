#include <Arduino.h>
#include <esp_now.h>
#include <display.h>
#include <buzzer.h>
#include <led.h>
#include <waterlevel.h>

EspNow espNow;
Display display;
Buzzer buzzer(D5);
Led led(LED_BUILTIN);

WaterLevelData *waterLevelData;

void setup()
{
    Serial.begin(115200);

    display.setup();
    buzzer.setup();
    led.setup();

    led.on();

    display.displayText("Water Level Monitor.");
    display.displayText("Initializing...", false);

    espNow.setup(&display);

    waterLevelData = &espNow.waterLevelData;

    led.off();
}

void loop()
{
    buzzer.update();
    led.blink();

    if (espNow.isLastUpdatedMoreThan(3))
    {
        led.blinkFor(100);
        // buzzer.start(1, 60000); // Continue beeping for 1 minute
    }
    else
    {
        led.blinkFor(1500);

        if (isLowLevel())
        {
            buzzer.start(2, 60000, 400); // Beep for 1 minute with 400ms cycle
        }
        else if (isHighLevel())
        {
            buzzer.start(3, 60000, 200); // Beep for 1 minute with 800ms cycle
        }
        else
        {
            buzzer.stop(true);

            if (waterLevelData->isPumpOn)
            {
                led.on(true);
            }
        }

        display.displayLevel(waterLevelData->level);
    }
}

bool isLowLevel()
{
    return waterLevelData->level != -1 && waterLevelData->level < waterLevelData->emptyThreshold && !waterLevelData->isPumpOn;
}

bool isHighLevel()
{
    return waterLevelData->level != -1 && waterLevelData->level > waterLevelData->fullThreshold && waterLevelData->isPumpOn;
}