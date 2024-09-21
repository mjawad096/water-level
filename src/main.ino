#include <Arduino.h>
#include <esp_now.h>
#include "distance.h"

EspNow espNow;
Distance distance;

unsigned long lastSendTime = 0;
unsigned long ledBlinkTime = 0;
bool ledState = false;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    espNow.setup();
    distance.setup();

    pinMode(LED_BUILTIN, LOW);
}

void loop()
{
    if (millis() - lastSendTime >= 5000)
    {
        double distanceValue = distance.getBestDistance();
        espNow.sendDistance(distanceValue);

        Serial.print("Distance sent: ");
        Serial.println(distanceValue);

        lastSendTime = millis();
        ledBlinkTime = millis();

        ledState = true;
        digitalWrite(LED_BUILTIN, HIGH);
    }

    if (ledState && millis() - ledBlinkTime >= 500)
    {
        digitalWrite(LED_BUILTIN, LOW);
        ledState = false;
    }
}