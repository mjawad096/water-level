#pragma once

#include "Esp.h"
#include "setting.h"

class Reset
{
private:
  Setting *settings;
  long lastResetPressTime;
  const int RESET_PIN = GPIO_NUM_13;

public:
  void setup(Setting *settings)
  {
    this->settings = settings;

    pinMode(RESET_PIN, INPUT_PULLUP);
    lastResetPressTime = millis();
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

    settings->reset();

    lastResetPressTime = millis();

    ESP.restart();
  }
};