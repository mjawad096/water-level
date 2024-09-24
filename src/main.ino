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

unsigned int continueInvalidLevelCount = 0;

unsigned long lastPingTime = 0;

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

    espNow.checkWifiConnection();

    webServer.sendWifiStatus();

    if (millis() - lastPingTime < settings.durationForPing * 1000)
    {
        return;
    }

    lastPingTime = millis();

    WaterLevelData levelData = waterLevel.getLevel();

    processWaterLevel(&levelData);
}

void processWaterLevel(WaterLevelData *levelData)
{
    if (levelData == nullptr)
    {
        Serial.println("Error: Null water level data received.");
        return;
    }

    // Filter out the noise
    if (level != -1 && !WaterLevel::isLastUpdatedMoreThan(1) && abs(level - levelData->level) >= 5 && continueInvalidLevelCount < 5)
    {
        levelData->level = level;
        levelData->distance = levelData->distance * -1;

        continueInvalidLevelCount++;
    }
    else
    {
        level = levelData->level;

        continueInvalidLevelCount = 0;
    }

    webServer.setWaterLevel(levelData);

    espNow.sendWaterLevel(levelData);

    display.displayLevel(levelData);

    rfSwitch.handleSwitchState(levelData);

    webServer.sendLastSwitchState();

    Serial.print("Level: ");
    Serial.print(levelData->level);
    Serial.print(", Distance: ");
    Serial.println(levelData->distance);
}