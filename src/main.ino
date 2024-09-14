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
    digitalWrite(LED_BUILTIN, HIGH);

    // if last updated is 5 min ago
    if (espNow.isLastUpdatedMoreThan(5))
    {
        digitalWrite(buzzerPin, HIGH);
    }
    else
    {

        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(buzzerPin, LOW);
    }

    delay(500);
}