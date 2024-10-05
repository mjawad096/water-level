#include "Arduino.h"
#include "LittleFS.h"
#include "Esp.h"
#include "ArduinoJson.h"
#include "WiFi.h"
#include "api_response.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include <HTTPClient.h>
#include "HTTPUpdate.h"
#include "wifi_connect.h"
#include "setting.h"
#include "switch.h"
#include "waterlevel.h"

#pragma once

class WebServer
{
private:
  AsyncWebServer *server;
  AsyncEventSource *events;
  Setting *settings;
  Switch *mySwitch;
  CurrentSensor *currentSensor;
  AsyncCallbackJsonWebHandler *saveSettingsHandler;

  bool rebootDevice = false;
  int waterLevel = 0;
  unsigned long lastWifiStatusSent = 0;
  unsigned long wifiStatusInterval = 15000;

public:
  WebServer()
  {
    server = new AsyncWebServer(80);
    events = new AsyncEventSource("/events");
  }

  ~WebServer()
  {
    delete server;
    delete events;
    delete saveSettingsHandler;
  }

  void setup(Setting *settings, CurrentSensor *currentSensor, Switch *mySwitch)
  {
    this->settings = settings;
    this->currentSensor = currentSensor;
    this->mySwitch = mySwitch;

    if (!LittleFS.begin())
    {
      Serial.println("Failed to mount file system");
      return;
    }

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

    setDefaultRoutes();
    setupSettingRoutes();
    setupStaticRoutes();
    setupSwitchRoutes();

    server->begin();

    Serial.println("Server started");
  }

  void setDefaultRoutes()
  {
    server->onNotFound(
        [](AsyncWebServerRequest *request)
        {
          sendApiResponse(request, request->method() == HTTP_OPTIONS ? 200 : 404);
        });

    server->on(
        "/water-level",
        [this](AsyncWebServerRequest *request)
        {
          JsonDocument doc;
          doc["level"] = waterLevel;

          sendApiResponse(request, 200, doc);
        });

    server->on(
        "/restart",
        [this](AsyncWebServerRequest *request)
        {
          sendApiResponse(request, 200, "Restarting device.");

          rebootDevice = true;
        });

    events->onConnect(
        [](AsyncEventSourceClient *client)
        {
          if (client->lastId())
          {
            Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
          }
          // send event with message "hello!", id current millis
          // and set reconnect delay to 1 second
          client->send("hello!", NULL, millis(), 10000);
        });

    server->addHandler(events);
  }

  void setupSettingRoutes()
  {
    server->on(
        "/settings",
        HTTP_GET,
        [this](AsyncWebServerRequest *request)
        {
          sendApiResponse(request, 200, settings->toJson());
        });

    server->on(
        "/reset",
        HTTP_POST,
        [this](AsyncWebServerRequest *request)
        {
          sendApiResponse(request, 200, "Resetting device.");

          settings->reset();
        });

    saveSettingsHandler = new AsyncCallbackJsonWebHandler(
        "/settings",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
          settings->fill(json);

          if (!settings->validate())
          {
            sendApiResponse(request, 400, settings->lastError ?: "Uknown error");
          }
          else
          {
            sendApiResponse(request, 200, "Settings saved");

            settings->save();
          }
        });

    server->addHandler(saveSettingsHandler);
  }

  void setupSwitchRoutes()
  {
    server->on(
        "/switch-on",
        HTTP_POST,
        [this](AsyncWebServerRequest *request)
        {
          bool isCurrentFlowing = currentSensor->isCurrentFlowing();
          String message = isCurrentFlowing ? "Pump is already ON." : "Switching ON.";

          sendApiResponse(request, 200, message);

          if (!isCurrentFlowing)
          {
            mySwitch->manualStart();
          }
        });

    server->on(
        "/switch-off",
        HTTP_POST,
        [this](AsyncWebServerRequest *request)
        {
          bool isCurrentFlowing = currentSensor->isCurrentFlowing();
          String message = !isCurrentFlowing ? "Pump is already OFF." : "Switching OFF.";

          sendApiResponse(request, 200, message);

          if (isCurrentFlowing)
          {
            mySwitch->manualStop();
          }
        });
  }

  void setupStaticRoutes()
  {
    server->on(
        "/", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(LittleFS, "/index.html", "text/html"); });

    server->on(
        "/index.css", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(LittleFS, "/index.css", "text/css"); });

    server->on(
        "/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(LittleFS, "/styles.css", "text/css"); });

    server->on(
        "/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(LittleFS, "/index.js", "text/javascript"); });
  }

  void checkForReboot()
  {
    if (rebootDevice)
    {
      delay(500);
      ESP.restart();
    }
  }
  void sendWifiStatus()
  {
    if (millis() - lastWifiStatusSent < wifiStatusInterval)
    {
      return;
    }

    lastWifiStatusSent = millis();

    int bufferSize = 20;
    char *buffer = new char[bufferSize];

    // Format the data into the allocated buffer
    snprintf(buffer, bufferSize, "{\"status\": %d}\n\n", WiFi.status() == WL_CONNECTED);

    events->send(buffer, "wifi_status", millis());

    delete[] buffer;
  }

  void setWaterLevel(WaterLevelData *levelData)
  {
    if (levelData == nullptr)
    {
      Serial.println("Error: Null water level data received.");
      return;
    }

    char *dataChars = levelData->formatForSSEvent();

    if (dataChars == nullptr)
    {
      Serial.println("Error: Memory allocation failed for SSE event.");

      return;
    }

    events->send(dataChars, "water_level_data", millis());

    delete[] dataChars;
  }
};