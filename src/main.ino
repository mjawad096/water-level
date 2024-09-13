#include "display.h"
#include "espnow.h"
#include "reset.h"
#include "setting.h"
#include "switch.h"
#include "webserver.h"

Display display;
EspNow espNow;
Reset reset;

Setting *settings;
Switch *rfSwitch;
WebServer *webServer;

WaterLevelData *waterLevelData;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    settings = new Setting();
    rfSwitch = new Switch();
    webServer = new WebServer();

    espNow.setup(settings);
    reset.setup(settings);
    display.setup();

    rfSwitch->setup(settings);
    webServer->setup(settings, rfSwitch);

    waterLevelData = EspNow::waterLevelData;
}

void loop()
{
    // Serial.println(settings->toString());

    webServer->checkForReset();
    webServer->checkForReboot();

    reset.checkForReset();

    processWaterLevel();

    delay(5000);
}

void processWaterLevel()
{
    webServer->setWaterLevel(waterLevelData);

    espNow.sendWaterLevel();

    display.displayLevel(waterLevelData->level);

    rfSwitch->checkForOpenState(waterLevelData->level);
    rfSwitch->checkForCloseState(waterLevelData->level);

    Serial.print("Level: ");
    Serial.print(waterLevelData->level);
    Serial.print(", Distance: ");
    Serial.println(waterLevelData->distance);
}