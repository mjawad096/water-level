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
  bool rebootDevice = false;
  int waterLevel = 0;

  AsyncWebServer *server;
  AsyncEventSource *events;
  Setting *settings;
  Switch *rfSwitch;

public:
  WebServer()
  {
    server = new AsyncWebServer(80);
    events = new AsyncEventSource("/events");
  }

  void setup(Setting *settings, Switch *rfSwitch)
  {
    this->settings = settings;
    this->rfSwitch = rfSwitch;

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

    AsyncCallbackJsonWebHandler *settingsHandler = new AsyncCallbackJsonWebHandler(
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

    server->addHandler(settingsHandler);
  }

  void setupSwitchRoutes()
  {
    server->on(
        "/switch-on",
        HTTP_POST,
        [this](AsyncWebServerRequest *request)
        {
          sendApiResponse(request, 200, "Switching on.");

          rfSwitch->sendSwitchState(true, true);
        });

    server->on(
        "/switch-off",
        HTTP_POST,
        [this](AsyncWebServerRequest *request)
        {
          sendApiResponse(request, 200, "Switching off.");

          rfSwitch->sendSwitchState(false, true);
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

  void setWaterLevel(WaterLevelData *levelData)
  {
    char *dataChars = levelData->formatForSSEvent();

    events->send(dataChars, "water_level_data", millis());

    // Clean up allocated memory
    delete[] dataChars;
  }
};