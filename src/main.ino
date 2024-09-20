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

    display.setup();

    display.displayText("Water Level Monitor.");
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

    webServer.setWaterLevel(levelData);

    espNow.sendWaterLevel(levelData);

    display.displayLevel(levelData);

    rfSwitch.handleSwitchState(levelData);

    Serial.print("Level: ");
    Serial.print(levelData->level);
    Serial.print(", Distance: ");
    Serial.println(levelData->distance);
}