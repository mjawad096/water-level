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

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    espNow.setup(&settings);
    reset.setup(&settings);
    waterLevel.setup(&settings);
    display.setup();

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

    webServer.setWaterLevel(levelData);
    espNow.sendWaterLevel(levelData);

    display.displayLevel(levelData->level);

    rfSwitch.checkForOpenState(levelData->level);
    rfSwitch.checkForCloseState(levelData->level);

    Serial.print("Level: ");
    Serial.print(levelData->level);
    Serial.print(", Distance: ");
    Serial.println(levelData->distance);
}