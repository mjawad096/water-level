#pragma once

#include <ESP8266WiFi.h>
#include <stdarg.h>
#include <waterlevel.h>

class TelnetLogger
{
private:
    static WiFiServer *server;
    static WiFiClient client;
    static bool started;
    static bool serialEnabled;

    static WaterLevelData *waterLevelData;

    // Private constructor to prevent instantiation
    TelnetLogger() {}

public:
    static void setup(WaterLevelData *waterLevelData)
    {
        if (started)
            return;

        uint16_t port = 23;

        server = new WiFiServer(port);
        server->begin();
        server->setNoDelay(true);

        started = true;

        Serial.println("[TelnetLogger] Started Telnet server");

        TelnetLogger::waterLevelData = waterLevelData;
    }

    static void handleClient()
    {
        if (!started)
            return;

        if (server->hasClient())
        {
            if (!client || !client.connected())
            {
                if (client)
                    client.stop();

                client = server->available();
                Serial.println("[TelnetLogger] Client connected");
                client.println("Welcome to ESP32 Telnet Logs");
            }
            else
            {
                WiFiClient newClient = server->available();
                newClient.stop(); // reject extra clients
            }
        }
    }

    static void log(const String &msg, bool newLine = true)
    {
        if (!started)
            return;

        String time = String(millis());

        if (waterLevelData != nullptr)
        {
            time = waterLevelData->time;
        }

        String line = "[" + time + "] " + msg;

        if (serialEnabled)
        {
            if (newLine)
            {
                Serial.println(line);
            }
            else
            {
                Serial.print(msg);
            }
        }

        if (client && client.connected())
        {
            if (newLine)
            {
                client.print("\r\n" + line);
            }
            else
            {
                client.print(msg);
            }
        }
    }

    static void logf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), format, args);

        va_end(args);

        log(buffer, true);
    }

    static bool isClientConnected()
    {
        return client && client.connected();
    }

    static void stopClient()
    {
        if (client)
            client.stop();
    }
};

WiFiServer *TelnetLogger::server = nullptr;
WiFiClient TelnetLogger::client;
WaterLevelData *TelnetLogger::waterLevelData = nullptr;
bool TelnetLogger::started = false;
bool TelnetLogger::serialEnabled = true;

#define LOG(x) TelnetLogger::log(x, false)        // Log without new line
#define LOGL(x) TelnetLogger::log(x)              // Log with new line
#define LOGF(...) TelnetLogger::logf(__VA_ARGS__) // Log with format