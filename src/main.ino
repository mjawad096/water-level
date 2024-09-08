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
}

void loop()
{
    reset.checkForReset();

    int level = waterLevel.getLevel();
    display.displayLevel(level);
    espNow.sendWaterLevel(level);

    delay(5000);
}