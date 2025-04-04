#include "display.h"
#include "espnow.h"
#include "reset.h"
#include "setting.h"
#include "switch.h"
#include "waterlevel.h"
#include "webserver.h"
#include "current_sensor.h"
#include "led.h"
#include "buzzer.h"

Display display;
EspNow espNow;
Reset reset;
WaterLevel waterLevel;

Setting settings;
Switch mySwitch;
WebServer webServer;
CurrentSensor currentSensor;

Led led;
Buzzer buzzer;

int level = -1;

unsigned int continueInvalidLevelCount = 0;

unsigned long lastPingTime = 0;

void setup()
{
    Serial.begin(115200);

    led.setup();
    buzzer.setup();

    display.setup();

    display.displayText("Initializing...", false);

    settings.load();

    espNow.setup(&settings, &display, &led);
    reset.setup(&settings);
    waterLevel.setup(&settings, &currentSensor);

    mySwitch.setup(&settings, &currentSensor, &buzzer);

    webServer.setup(&settings, &currentSensor, &mySwitch);

    led.off();
    buzzer.stop(true);
}

void loop()
{
    buzzer.update();
    led.blink();

    currentSensor.readCurrent();

    mySwitch.checkForLevel(level);
    mySwitch.checkForManualSwitchRequested();
    mySwitch.checkForInternalSwitchChange();

    webServer.checkForReboot();

    reset.checkForReset();

    espNow.checkWifiConnection();

    webServer.sendWifiStatus();

    if (WaterLevel::isLastUpdatedMoreThan(3))
    {
        led.blinkFor(100);
    }
    else if (currentSensor.isCurrentFlowing())
    {
        led.on(true);
    }
    else
    {
        led.blinkFor(1500);
    }

    if (isLowLevel())
    {
        buzzer.start(2, 60000, 400);
    }
    else if (isHighLevel())
    {
        buzzer.start(3, 60000, 200);
    }
    else
    {
        buzzer.stop(true);
    }

    if (millis() - lastPingTime < settings.durationForPing * 1000)
    {
        return;
    }

    lastPingTime = millis();

    WaterLevelData levelData = waterLevel.getLevel();

    processWaterLevel(&levelData);

    mySwitch.handlePendingState();
}

bool isLowLevel()
{
    return level != -1 && level < settings.emptyThreshold && !currentSensor.isCurrentFlowing();
}

bool isHighLevel()
{
    return level != -1 && level > settings.fullThreshold && currentSensor.isCurrentFlowing();
}

void processWaterLevel(WaterLevelData *levelData)
{
    if (levelData == nullptr)
    {
        Serial.println("Error: Null water level data received.");
        return;
    }

    // Filter out the noise
    if (level != -1 && !WaterLevel::isLastUpdatedMoreThan(1) && (level - levelData->level) >= 4 && continueInvalidLevelCount < 5)
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
}