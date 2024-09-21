#include <Arduino.h>
#include <esp_now.h>
#include "distance.h"

EspNow espNow;
Distance distance;

unsigned long lastSendTime = 0;

const int LED_OFF = HIGH;
const int LED_ON = LOW;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    espNow.setup();
    distance.setup();

    digitalWrite(LED_BUILTIN, LED_OFF);
}

void loop()
{
    if (millis() - lastSendTime >= 3000)
    {
        digitalWrite(LED_BUILTIN, LED_ON);

        unsigned long readStart = millis();

        double distanceValue = distance.getBestDistance();

        Serial.print("Sensor Time:");
        Serial.println((millis() - readStart) / 1000);

        espNow.sendDistance(distanceValue);

        Serial.print("Distance sent: ");
        Serial.println(distanceValue);

        lastSendTime = millis();

        digitalWrite(LED_BUILTIN, LED_OFF);
    }
}