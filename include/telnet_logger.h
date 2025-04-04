#pragma once

#include <WiFi.h>
#include <stdarg.h>
#include <time_manager.h>

class TelnetLogger
{
private:
    static WiFiServer server;
    static WiFiClient client;
    static bool started;
    static bool serialEnabled;

    // Private constructor to prevent instantiation
    TelnetLogger() {}

public:
    static void setup(uint16_t port = 23)
    {
        if (started)
            return;

        server = WiFiServer(port);
        server.begin();
        server.setNoDelay(true);

        started = true;

        Serial.println("[TelnetLogger] Started Telnet server");

        serialEnabled = false;
    }

    static void handleClient()
    {
        if (!started)
            return;

        if (server.hasClient())
        {
            if (!client || !client.connected())
            {
                if (client)
                    client.stop();

                client = server.available();
                Serial.println("[TelnetLogger] Client connected");
                client.println("Welcome to ESP32 Telnet Logs");
            }
            else
            {
                WiFiClient newClient = server.available();
                newClient.stop(); // reject extra clients
            }
        }
    }

    static void log(const String &msg, bool newLine = true)
    {
        String line = "[" + TimeManager::getFormattedTime() + "] " + msg;

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
                client.print(line + "\r\n");
            }
            else
            {
                client.print(msg);
            }
        }
    }

    static void logf(const char *format, ...)
    {
        char buffer[128];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        log(String(buffer));
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

WiFiServer TelnetLogger::server;
WiFiClient TelnetLogger::client;
bool TelnetLogger::started = false;
bool TelnetLogger::serialEnabled = true;