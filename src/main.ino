#include <Arduino.h>
#include <esp_now.h>
#include <display.h>
#include <buzzer.h>

EspNow espNow;
Display display;
Buzzer buzzer(D5);

const int LED_ON = LOW;
const int LED_OFF = HIGH;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

    espNow.setup();
    display.setup();
    buzzer.setup();
}

void loop()
{
    if (espNow.isLastUpdatedMoreThan(5))
    {
        digitalWrite(LED_BUILTIN, LED_OFF);
        delay(100);

        buzzer.start(1, 60000); // Start the buzzer for 60 seconds
        digitalWrite(LED_BUILTIN, LED_ON);
        delay(100);
    }
    else
    {
        if (espNow.waterLevel < 25)
        {
            buzzer.start(2, 60000, 300); // Start the buzzer for 60 seconds
        }
        else
        {
            buzzer.stop(true);
        }

        display.displayLevel(espNow.waterLevel);

        digitalWrite(LED_BUILTIN, LED_ON);
        delay(2000);

        digitalWrite(LED_BUILTIN, LED_OFF);
        delay(1000);
    }

    buzzer.update();
}