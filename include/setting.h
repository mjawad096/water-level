#include <ArduinoJson.h>
#include <Preferences.h>

#pragma once

class Setting
{

private:
    Preferences preferences;

public:
    long durationForPing = 300;
    long topEndFromDevice = 0;
    long bottomEndFromDevice = 250;
    long delayStartSwitch = 10; // minutes
    long delayStopSwitch = 10;  // minutes
    long fullThreshold = 99;
    long emptyThreshold = 5;
    bool autoOffOnFull = false;
    bool autoOnOnEmpty = false;

    String lastError = "";

    Setting()
    {
    }

    Setting(const JsonDocument &obj)
    {
        durationForPing = obj["durationForPing"].as<long>();
        topEndFromDevice = obj["topEndFromDevice"].as<long>();
        bottomEndFromDevice = obj["bottomEndFromDevice"].as<long>();
        delayStartSwitch = obj["delayStartSwitch"].as<long>();
        delayStopSwitch = obj["delayStopSwitch"].as<long>();
        fullThreshold = obj["fullThreshold"].as<long>();
        emptyThreshold = obj["emptyThreshold"].as<long>();
        autoOffOnFull = obj["autoOffOnFull"].as<bool>();
        autoOnOnEmpty = obj["autoOnOnEmpty"].as<bool>();
    }

    void setup()
    {
        preferences.begin("settings", false);

        Serial.println("Pref Settings loaded");

        durationForPing = preferences.getLong("durationForPing", 300);
        topEndFromDevice = preferences.getLong("topEndFromDevice", 0);
        bottomEndFromDevice = preferences.getLong("bottomEndFromDevice", 250);
        delayStartSwitch = preferences.getLong("delayStartSwitch", 10);
        delayStopSwitch = preferences.getLong("delayStopSwitch", 10);
        fullThreshold = preferences.getLong("fullThreshold", 99);
        emptyThreshold = preferences.getLong("emptyThreshold", 5);
        autoOffOnFull = preferences.getBool("autoOffOnFull", false);
        autoOnOnEmpty = preferences.getBool("autoOnOnEmpty", false);

        preferences.end();
    }

    void save()
    {
        preferences.begin("settings", false);

        preferences.putLong("durationForPing", durationForPing);
        preferences.putLong("topEndFromDevice", topEndFromDevice);
        preferences.putLong("bottomEndFromDevice", bottomEndFromDevice);
        preferences.putLong("delayStartSwitch", delayStartSwitch);
        preferences.putLong("delayStopSwitch", delayStopSwitch);
        preferences.putLong("fullThreshold", fullThreshold);
        preferences.putLong("emptyThreshold", emptyThreshold);
        preferences.putBool("autoOffOnFull", autoOffOnFull);
        preferences.putBool("autoOnOnEmpty", autoOnOnEmpty);

        preferences.end();
    }

    void reset()
    {
        durationForPing = 300;
        topEndFromDevice = 0;
        bottomEndFromDevice = 250;
        delayStartSwitch = 10;
        delayStopSwitch = 10;
        fullThreshold = 99;
        emptyThreshold = 5;
        autoOffOnFull = false;
        autoOnOnEmpty = false;

        save();
    }

    String toString()
    {
        String data;
        JsonDocument doc;

        doc["durationForPing"] = durationForPing;
        doc["topEndFromDevice"] = topEndFromDevice;
        doc["bottomEndFromDevice"] = bottomEndFromDevice;
        doc["delayStartSwitch"] = delayStartSwitch;
        doc["delayStopSwitch"] = delayStopSwitch;
        doc["fullThreshold"] = fullThreshold;
        doc["emptyThreshold"] = emptyThreshold;
        doc["autoOffOnFull"] = autoOffOnFull;
        doc["autoOnOnEmpty"] = autoOnOnEmpty;

        serializeJson(doc, data);

        return data;
    }

    bool validate()
    {
        return true;
    }
};