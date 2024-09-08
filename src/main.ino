#include "display.h"
#include "espnow.h"
#include "reset.h"
#include "setting.h"
#include "switch.h"
#include "waterlevel.h"

Display display;
EspNow espNow;
Reset reset;
Setting settings;
Switch rfSwitch;
WaterLevel waterLevel;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    espNow.setup();
    reset.setup();
    settings.setup();
    rfSwitch.setup();
    waterLevel.setup();

    display.setup();
}

void loop()
{
    reset.checkForReset();

    int level = waterLevel.getLevel();

    display.displayLevel(level);
    espNow.sendWaterLevel(level);

    rfSwitch.checkForOpenState(level);
    rfSwitch.checkForCloseState(level);

    Serial.print("Level: ");
    Serial.println(level);

    delay(5000);
}