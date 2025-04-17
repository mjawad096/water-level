#pragma once

#include <setting.h>
#include "current_sensor.h"
#include "buzzer.h"
#include <logger.h>

class Switch
{
private:
    Setting *settings;
    CurrentSensor *currentSensor;
    Buzzer *buzzer;
    Database *database;

    int internalPinState = LOW;

    const int externalPin = GPIO_NUM_27;
    const int internalPin = GPIO_NUM_32;

    long lastSentOnTime = 0;
    long lastSentOffTime = 0;

    int manualSwitchRequested = -1;

    // Debounce timing variables for interal pin
    long lastDebounceTime = 0;
    const long debounceDelay = 100;

    int pendingState = -1;

    // Debounce Current flow for autometic ON/OFF due to noice in current sensor
    int currentFlowingCountOpen = 0;
    int currentFlowingCountClose = 0;

public:
    static RTC_DATA_ATTR int externalPinState;

    void setup(Setting *settings, CurrentSensor *currentSensor, Buzzer *buzzer, Database *database)
    {
        this->settings = settings;
        this->currentSensor = currentSensor;
        this->buzzer = buzzer;
        this->database = database;

        pinMode(externalPin, OUTPUT);
        pinMode(internalPin, INPUT_PULLUP);

        internalPinState = digitalRead(internalPin);

        int savedState = database->loadExternalSwitchState();
        externalPinState = savedState != -1 ? savedState : LOW;
        digitalWrite(externalPin, externalPinState);
    }

    void handlePendingState()
    {
        LOGF("Handling pending State: %d, Current state: %d, Current: %.2f ", pendingState, currentSensor->isCurrentFlowing(), currentSensor->getCurrent());

        if (pendingState == -1 || pendingState == currentSensor->isCurrentFlowing())
        {
            if (pendingState != -1)
            {
                LOGL("Switch state is already in the requested state: " + String(pendingState));
            }

            pendingState = -1;

            return;
        }

        int newExternalPinState = externalPinState == HIGH ? LOW : HIGH;

        digitalWrite(externalPin, newExternalPinState);

        externalPinState = newExternalPinState;

        database->saveExternalSwitchState(externalPinState);

        LOGL("Switch State changed to " + String(pendingState));

        pendingState = -1;
    }

    void checkForInternalSwitchChange()
    {
        int newInternalPinState = digitalRead(internalPin);

        if (internalPinState == newInternalPinState)
        {
            return;
        }

        long currentTime = millis();
        if (currentTime - lastDebounceTime <= debounceDelay)
        {
            return;
        }

        lastDebounceTime = currentTime;

        internalPinState = newInternalPinState;

        LOGL("Internal switch state changed: " + String(internalPinState));

        pendingState = currentSensor->isCurrentFlowing() ? false : true;
    }

    void checkForManualSwitchRequested()
    {
        if (manualSwitchRequested == -1)
        {
            return;
        }

        pendingState = manualSwitchRequested;

        LOGL("Manual switch requested: " + String(manualSwitchRequested));

        manualSwitchRequested = -1;
    }

    void checkForLevel(int level)
    {
        checkForOpenState(level);
        checkForCloseState(level);
    }

    void checkForOpenState(int level)
    {
        if (!settings->autoOnOnEmpty || level == -1 || level > settings->emptyThreshold)
        {
            currentFlowingCountOpen = 0;
            return;
        }

        // If current is still flowing, reset the counter
        if (currentSensor->isCurrentFlowing())
        {
            currentFlowingCountOpen = 0;
            return;
        }

        // If current is not flowing, increment the counter
        currentFlowingCountOpen++;

        // Wait until we've seen 3 consecutive "no current" checks
        if (currentFlowingCountOpen < 3)
        {
            return;
        }

        if (millis() - lastSentOnTime < (settings->delayStartSwitch * 1000))
        {
            return;
        }

        pendingState = true;
        lastSentOnTime = millis();

        LOGL("Tank empty, Sent Switch state to ON");
    }

    void checkForCloseState(int level)
    {
        if (!settings->autoOffOnFull || level == -1 || level < settings->fullThreshold)
        {
            currentFlowingCountClose = 0;
            return;
        }

        // If current is flowing, increment counter
        if (currentSensor->isCurrentFlowing())
        {
            currentFlowingCountClose++;
        }
        else
        {
            currentFlowingCountClose = 0;
            return;
        }

        // Wait until we've seen 3 consecutive "current is flowing" checks
        if (currentFlowingCountClose < 3)
        {
            return;
        }

        // Respect delay before sending OFF
        if (millis() - lastSentOffTime < (settings->delayStopSwitch * 1000))
        {
            return;
        }

        // Conditions met — turn pump OFF
        pendingState = false;
        lastSentOffTime = millis();

        LOGL("Tank full, Sent Switch state to OFF");
    }

    void manualStart()
    {
        manualSwitchRequested = 1;
    }

    void manualStop()
    {
        manualSwitchRequested = 0;
    }
};

RTC_DATA_ATTR int Switch::externalPinState = LOW;