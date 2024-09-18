#include <ArduinoJson.h>
#include <Preferences.h>

#pragma once

class Setting
{

private:
    Preferences preferences;

public:
    String apSSID = "";
    String apPassword = "";

    long durationForPing = 5; // seconds
    long topEndFromDevice = 0;
    long bottomEndFromDevice = 250;
    long delayStartSwitch = 10; // minutes
    long delayStopSwitch = 10;  // minutes
    long fullThreshold = 99;
    long emptyThreshold = 5;
    bool autoOffOnFull = false;
    bool autoOnOnEmpty = false;

    String lastError = "";

    void load()
    {
        preferences.begin("settings", false);

        Serial.println("Pref Settings loaded");

        apSSID = preferences.getString("apSSID", "");
        apPassword = preferences.getString("apPassword", "");

        durationForPing = preferences.getLong("durationForPing", 5);
        topEndFromDevice = preferences.getLong("topFromDevice", 0);
        bottomEndFromDevice = preferences.getLong("btmFromDevice", 250);
        delayStartSwitch = preferences.getLong("delayStartSwtch", 10);
        delayStopSwitch = preferences.getLong("delayStopSwtch", 10);
        fullThreshold = preferences.getLong("fullThreshold", 99);
        emptyThreshold = preferences.getLong("emptyThreshold", 5);
        autoOffOnFull = preferences.getBool("autoOffOnFull", false);
        autoOnOnEmpty = preferences.getBool("autoOnOnEmpty", false);

        preferences.end();
    }

    void fill(const JsonVariant &json)
    {
        JsonDocument doc;
        doc.set(json);

        fill(doc);
    }

    void fill(const JsonDocument &obj)
    {
        apSSID = obj["apSSID"].as<String>();
        apPassword = obj["apPassword"].as<String>();

        durationForPing = obj["durationForPing"].as<long>();
        topEndFromDevice = obj["topEndFromDevice"].as<long>();
        bottomEndFromDevice = obj["bottomEndFromDevice"].as<long>();
        delayStartSwitch = obj["delayStartSwitch"].as<long>();
        delayStopSwitch = obj["delayStopSwitch"].as<long>();
        fullThreshold = obj["fullThreshold"].as<long>();
        emptyThreshold = obj["emptyThreshold"].as<long>();
        autoOffOnFull = obj["autoOffOnFull"].as<bool>();
        autoOnOnEmpty = obj["autoOnOnEmpty"].as<bool>();

        if (durationForPing == 0)
        {
            durationForPing = 5;
        }
    }

    void save()
    {
        preferences.begin("settings", false);

        preferences.putString("apSSID", apSSID);
        preferences.putString("apPassword", apPassword);

        preferences.putLong("durationForPing", durationForPing);
        preferences.putLong("topFromDevice", topEndFromDevice);
        preferences.putLong("btmFromDevice", bottomEndFromDevice);
        preferences.putLong("delayStartSwtch", delayStartSwitch);
        preferences.putLong("delayStopSwtch", delayStopSwitch);
        preferences.putLong("fullThreshold", fullThreshold);
        preferences.putLong("emptyThreshold", emptyThreshold);
        preferences.putBool("autoOffOnFull", autoOffOnFull);
        preferences.putBool("autoOnOnEmpty", autoOnOnEmpty);

        preferences.end();
    }

    void reset()
    {
        apSSID = "";
        apPassword = "";

        durationForPing = 5;
        topEndFromDevice = 0;
        bottomEndFromDevice = 250;
        delayStartSwitch = 10;
        delayStopSwitch = 10;
        fullThreshold = 99;
        emptyThreshold = 5;
        autoOffOnFull = false;
        autoOnOnEmpty = false;

        save();

        Serial.println("Device Reset");
    }

    JsonDocument toJson()
    {
        JsonDocument doc;

        doc["apSSID"] = apSSID;
        doc["apPassword"] = apPassword;

        doc["durationForPing"] = durationForPing;
        doc["topEndFromDevice"] = topEndFromDevice;
        doc["bottomEndFromDevice"] = bottomEndFromDevice;
        doc["delayStartSwitch"] = delayStartSwitch;
        doc["delayStopSwitch"] = delayStopSwitch;
        doc["fullThreshold"] = fullThreshold;
        doc["emptyThreshold"] = emptyThreshold;
        doc["autoOffOnFull"] = autoOffOnFull;
        doc["autoOnOnEmpty"] = autoOnOnEmpty;

        return doc;
    }

    String toString()
    {
        String data;

        serializeJson(toJson(), data);

        return data;
    }

    bool validate()
    {
        return true;
    }
};