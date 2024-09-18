#include <Arduino.h>
#include <esp_now.h>
#include <display.h>
#include <buzzer.h>
#include <led.h>

EspNow espNow;
Display display;
Buzzer buzzer(D5);
Led led(LED_BUILTIN);

void setup()
{
    Serial.begin(115200);

    espNow.setup();
    display.setup();
    buzzer.setup();
    led.setup();
}

void loop()
{
    buzzer.update();
    led.blink();

    if (espNow.isLastUpdatedMoreThan(5))
    {
        led.blinkFor(100);
        buzzer.start(1, 60000); // Continue beeping for 1 minute
    }
    else
    {
        led.blinkFor(1500);

        if (espNow.waterLevel < 5)
        {
            buzzer.start(2, 60000, 400); // Beep for 1 minute with 400ms cycle
        }
        else
        {
            buzzer.stop(true);
        }

        display.displayLevel(espNow.waterLevel);
    }
}