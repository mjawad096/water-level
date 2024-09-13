#include <Arduino.h>
#include <esp_now.h>
#include "distance.h"

EspNow espNow;
Distance distance;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    espNow.setup();
    distance.setup();
}

void loop()
{
    double bestDistance = random(1, 101);
    // double bestDistance = distance.getBestDistance();

    espNow.sendDistance(bestDistance);

    delay(5000);
}