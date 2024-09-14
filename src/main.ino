#include <Arduino.h>
#include <esp_now.h>

EspNow espNow;

const int buzzerPin = D5;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(buzzerPin, OUTPUT);

    Serial.begin(115200);

    espNow.setup();
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
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);

        digitalWrite(buzzerPin, LOW);

        delay(1000);
    }
}