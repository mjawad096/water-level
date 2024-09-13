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
#include "waterlevel.h"
#include "switch.h"

#pragma once

class WebServer
{
private:
  bool rebootDevice = false;
  bool resetDevice = false;
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
          if (request->method() == HTTP_OPTIONS)
          {
            request->send(200);
          }
          else
          {
            request->send(404);
          }
        });

    server->on(
        "/water-level",
        [this](AsyncWebServerRequest *request)
        {
          JsonDocument doc;
          doc["level"] = waterLevel;

          String response;
          serializeJson(doc, response);
          request->send(200, "application/json", response);
        });

    server->on(
        "/restart",
        [this](AsyncWebServerRequest *request)
        {
          request->send(200, "application/json", apiResponse("Restarting device."));
          request->client()->close();
          rebootDevice = true;
        });

    server->on(
        "/reset",
        HTTP_POST,
        [this](AsyncWebServerRequest *request)
        {
          request->send(200, "application/json", apiResponse("Resetting device."));
          request->client()->close();
          resetDevice = true;
        });

    events->onConnect([](AsyncEventSourceClient *client)
                      {
      if(client->lastId()){
        Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
      }
      // send event with message "hello!", id current millis
      // and set reconnect delay to 1 second
      client->send("hello!", NULL, millis(), 10000); });

    server->addHandler(events);
  }

  void setupSettingRoutes()
  {
    server->on(
        "/settings",
        HTTP_GET,
        [this](AsyncWebServerRequest *request)
        {
          request->send(200, "application/json", settings->toString());
        });

    AsyncCallbackJsonWebHandler *settingsHandler = new AsyncCallbackJsonWebHandler(
        "/settings",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
          JsonDocument doc;
          doc.set(json);
          settings->fill(doc);

          if (!settings->validate())
          {
            request->send(400, "application/json", apiResponse(settings->lastError ?: "Uknown error"));
          }
          else
          {
            settings->save();
            rebootDevice = true;
            request->send(200, "application/json", apiResponse("Settings saved"));
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
          request->send(200, "application/json", apiResponse("Switching on."));
          request->client()->close();

          rfSwitch->sendSwitchState(true, true);
        });

    server->on(
        "/switch-off",
        HTTP_POST,
        [this](AsyncWebServerRequest *request)
        {
          request->send(200, "application/json", apiResponse("Switching off."));
          request->client()->close();

          rfSwitch->sendSwitchState(false, true);
        });
  }

  void setupStaticRoutes()
  {
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, "/index.html", "text/html"); });

    server->on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, "/index.css", "text/css"); });

    server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, "/styles.css", "text/css"); });

    server->on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
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

  void checkForReset()
  {
    if (resetDevice)
    {
      delay(500);
      settings->reset();
      rebootDevice = true;
    }
  }

  void setWaterLevel(WaterLevelData levelData)
  {
    char *dataChars = levelData.formatForSSEvent();

    events->send(dataChars, "water_level_data", millis());

    // Clean up allocated memory
    delete[] dataChars;
  }
};