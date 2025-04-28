#include "espnow.h"
#include "reset.h"
#include "webserver.h"
#include "database.h"

Display display;
EspNow espNow;
Reset reset;
WaterLevel waterLevel;

Setting settings;
Switch mySwitch;
WebServer webServer;
CurrentSensor currentSensor;

Database database;

Led led;
Buzzer buzzer;

int level = -1;
bool isPumpOn = false;

unsigned int continueInvalidLevelCount = 0;

unsigned long lastPingTime = 0;

void setup()
{
    Serial.begin(115200);

    Logger::setup(&database);

    led.setup();
    buzzer.setup();

    display.setup();
    display.displayText("Initializing...", false);

    settings.load();
    display.displayText("Settings loaded...", false);

    espNow.setup(&settings, &display, &led, &database);

    LOGL("Settings loaded...");

    reset.setup(&settings);
    waterLevel.setup(&settings, &currentSensor);

    mySwitch.setup(&settings, &currentSensor, &buzzer);

    webServer.setup(&settings, &currentSensor, &mySwitch, &database);

    database.logResetReason();

    led.off();
    buzzer.stop(true);
}

void loop()
{
    TelnetLogger::handleClient();
    TimeManager::updateTime();
    OtaManager::handle();

    buzzer.update();
    led.blink();

    currentSensor.readCurrent();

    mySwitch.checkForLevel(level);
    mySwitch.checkForManualSwitchRequested();
    mySwitch.checkForInternalSwitchChange();

    if (isPumpOn != currentSensor.isCurrentFlowing())
    {
        processWaterLevel();
    }

    isPumpOn = currentSensor.isCurrentFlowing();

    webServer.checkForReboot();

    reset.checkForReset();

    espNow.checkWifiConnection();

    webServer.sendWifiStatus();

    if (currentSensor.isCurrentFlowing())
    {
        led.on(true);
    }
    else if (WaterLevel::isLastUpdatedMoreThan(3))
    {
        led.blinkFor(100);
    }
    else
    {
        led.blinkFor(1500);
    }

    if (isLowLevel() && settings.alarmEnabled)
    {
        buzzer.start(2, 60000, 400); // 1 minute
    }
    else if (isHighLevel() && settings.alarmEnabled)
    {
        buzzer.start(3, 3600000, 200); // 1 hour
    }
    else
    {
        buzzer.stop(true);
    }

    display.handlePixelRefresher();

    espNow.checkIfSentFailureForPeer2();

    if (millis() - lastPingTime < settings.durationForPing * 1000)
    {
        return;
    }

    lastPingTime = millis();

    processWaterLevel();

    mySwitch.handlePendingState();

    database.deleteOldFiles();
}

bool isLowLevel()
{
    return level != -1 && level < settings.emptyThreshold && !currentSensor.isCurrentFlowing();
}

bool isHighLevel()
{
    return level != -1 && level > settings.fullThreshold && currentSensor.isCurrentFlowing();
}

void processWaterLevel()
{
    WaterLevelData levelData = waterLevel.getLevel();

    // Filter out the noise
    if (level != -1 && !WaterLevel::isLastUpdatedMoreThan(1) && abs(level - levelData.level) >= 4 && continueInvalidLevelCount < 9)
    {
        levelData.level = level;
        levelData.distance = levelData.distance * -1;

        continueInvalidLevelCount++;
    }
    else
    {
        level = levelData.level;

        continueInvalidLevelCount = 0;
    }

    webServer.setWaterLevel(&levelData);

    espNow.sendWaterLevel(&levelData);

    display.displayLevel(&levelData);

    database.saveLevelEntry(&levelData);
}