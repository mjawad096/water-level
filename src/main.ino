#include "display.h"
#include "espnow.h"
#include "reset.h"
#include "setting.h"
#include "switch.h"
#include "waterlevel.h"
#include "webserver.h"

Display display;
EspNow espNow;
Reset reset;
WaterLevel waterLevel;

Setting settings;
Switch rfSwitch;
WebServer webServer;

int level = -1;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    display.setup();

    display.displayText("Initializing...", false);

    settings.load();

    espNow.setup(&settings, &display);
    reset.setup(&settings);
    waterLevel.setup(&settings);

    rfSwitch.setup(&settings);
    webServer.setup(&settings, &rfSwitch);
}

void loop()
{
    // Serial.println(settings.toString());

    webServer.checkForReboot();

    reset.checkForReset();

    WaterLevelData levelData = waterLevel.getLevel();

    processWaterLevel(&levelData);

    delay(settings.durationForPing * 1000);
}

void processWaterLevel(WaterLevelData *levelData)
{
    if (levelData == nullptr)
    {
        Serial.println("Error: Null water level data received.");
        return;
    }

    // check if new level has a lot diff more than 10% then use old level
    if (level != -1 && abs(level - levelData->level) > 10)
    {
        levelData->level = level;
    }
    else
    {
        level = levelData->level;
    }

    webServer.setWaterLevel(levelData);

    espNow.sendWaterLevel(levelData);

    display.displayLevel(levelData);

    rfSwitch.handleSwitchState(levelData);

    Serial.print("Level: ");
    Serial.print(levelData->level);
    Serial.print(", Distance: ");
    Serial.println(levelData->distance);
}