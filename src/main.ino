#include <Arduino.h>
#include <esp_now.h>
#include <display.h>

EspNow espNow;
Display display;

const int buzzerPin = D5;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(buzzerPin, OUTPUT);

    Serial.begin(115200);

    espNow.setup();
    display.setup();
}

void loop()
{

    if (espNow.isLastUpdatedMoreThan(2))
    {
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);

        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(buzzerPin, HIGH);

        delay(100);
    }
    else
    {
        digitalWrite(buzzerPin, LOW);

        display.displayLevel(espNow.waterLevel);

        digitalWrite(LED_BUILTIN, HIGH);
        delay(2000);

        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
    }
}