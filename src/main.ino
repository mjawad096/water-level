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
    }
    else if (waterLevelData->isPumpOn)
    {
        led.on(true);
    }
    else
    {
        led.blinkFor(1500);
    }

    if (isLowLevel())
    {
        buzzer.start(2, 3600000, 400); // 1 hour
    }
    else if (isHighLevel())
    {
        buzzer.start(3, 3600000, 200); // 1 hour
    }
    else
    {
        buzzer.stop(true);
    }

    display.displayLevel(waterLevelData->level);
}

bool isLowLevel()
{
    return waterLevelData->level != -1 && waterLevelData->level < waterLevelData->emptyThreshold && !waterLevelData->isPumpOn;
}

bool isHighLevel()
{
    return waterLevelData->level != -1 && waterLevelData->level > waterLevelData->fullThreshold && waterLevelData->isPumpOn;
}