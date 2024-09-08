#include "Esp.h"
#include "Setting.h"

#pragma once

class Reset
{
private:
  Setting settings;
  long lastResetPressTime;
  const int RESET_PIN = 4;

public:
  void setup()
  {
    pinMode(RESET_PIN, INPUT_PULLUP);
    lastResetPressTime = millis();

    settings.setup();
  }

  void checkForReset()
  {
    if (digitalRead(RESET_PIN) != LOW)
    {
      lastResetPressTime = millis();
      return void();
    }

    delay(100);
    long totalResetPressTimeInSeconds = (millis() - lastResetPressTime) / 1000;
    if (totalResetPressTimeInSeconds < 5)
    {
      return void();
    }

    settings.reset();

    lastResetPressTime = millis();
    Serial.println("Device Reset");

    ESP.restart();
  }
};