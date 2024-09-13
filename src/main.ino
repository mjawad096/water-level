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
    // if last updated is 5 min ago
    if (espNow.isLastUpdatedMoreThan(5))
    {
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(buzzerPin, HIGH);
    }
    else
    {
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(buzzerPin, LOW);
    }
}