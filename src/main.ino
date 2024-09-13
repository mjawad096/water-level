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

Setting *settings;
Switch *rfSwitch;
WebServer *webServer;

int level = 0;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    settings = new Setting();
    rfSwitch = new Switch();
    webServer = new WebServer();

    espNow.setup(settings);
    reset.setup(settings);
    waterLevel.setup(settings);
    display.setup();

    rfSwitch->setup(settings);
    webServer->setup(settings, rfSwitch);
}

void loop()
{
    // Serial.println(settings->toString());

    webServer->checkForReset();
    webServer->checkForReboot();

    reset.checkForReset();

    processWaterLevel(waterLevel.getLevel());

    delay(settings->durationForPing * 1000);
}

void processWaterLevel(WaterLevelData levelData)
{

    webServer->setWaterLevel(levelData);
    espNow.sendWaterLevel(levelData);

    level = levelData.level;

    display.displayLevel(level);

    rfSwitch->checkForOpenState(level);
    rfSwitch->checkForCloseState(level);

    Serial.print("Level: ");
    Serial.print(levelData.level);
    Serial.print(", Distance: ");
    Serial.println(levelData.distance);
}