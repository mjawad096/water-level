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
Setting settings;
Switch rfSwitch;
WaterLevel waterLevel;
WebServer *webServer = new WebServer();

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    espNow.setup();
    webServer->setup();
    reset.setup();
    settings.setup();
    rfSwitch.setup();
    waterLevel.setup();

    display.setup();
}

void loop()
{
    // Serial.println(settings.toString());

    // Check and reboot if requested
    if (webServer->rebootRequested())
    {
        delay(500);
        ESP.restart();
    }

    reset.checkForReset();

    int level = waterLevel.getLevel();

    webServer->setWaterLevel(level);

    display.displayLevel(level);
    espNow.sendWaterLevel(level);

    rfSwitch.checkForOpenState(level);
    rfSwitch.checkForCloseState(level);

    Serial.print("Level: ");
    Serial.println(level);

    delay(settings.durationForPing * 1000);
}