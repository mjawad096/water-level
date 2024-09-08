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

#pragma once

class WebServer
{
private:
  AsyncWebServer *server;
  Setting settings;
  bool rebootDevice = false;
  int waterLevel = 0;

public:
  WebServer()
  {
    server = new AsyncWebServer(80);
  }

  void setup()
  {
    settings.setup();

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
    setupAdminPanelRoutes();

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
        "/",
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
        "/networks/scan",
        [this](AsyncWebServerRequest *request)
        {
          WifiConnect wifiConnect;
          request->send(200, "application/json", wifiConnect.getAvailableNetworks(true));
          request->client()->close();
        });

    server->on(
        "/networks",
        [this](AsyncWebServerRequest *request)
        {
          WifiConnect wifiConnect;
          request->send(200, "application/json", wifiConnect.getAvailableNetworks());
          request->client()->close();
        });
  }

  void setupSettingRoutes()
  {
    server->on(
        "/settings",
        HTTP_GET,
        [this](AsyncWebServerRequest *request)
        {
          request->send(200, "application/json", settings.toString());
        });

    AsyncCallbackJsonWebHandler *settingsHandler = new AsyncCallbackJsonWebHandler(
        "/settings",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
          JsonDocument doc;
          doc.set(json);
          settings.fill(doc);

          if (!settings.validate())
          {
            request->send(400, "application/json", apiResponse(settings.lastError ?: "Uknown error"));
          }
          else
          {
            settings.save();
            rebootDevice = true;
            request->send(200, "application/json", apiResponse("Settings saved"));
          }
        });

    server->addHandler(settingsHandler);
  }

  void setupAdminPanelRoutes()
  {
    server->on("/admin", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, "/index.html", "text/html"); });

    server->on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, "/index.css", "text/css"); });

    server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, "/styles.css", "text/css"); });

    server->on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, "/index.js", "text/javascript"); });
  }

  bool rebootRequested()
  {
    return rebootDevice;
  }

  void setWaterLevel(int level) {
    waterLevel = level;
  }
};