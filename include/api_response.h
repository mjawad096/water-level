#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#pragma once

void sendApiResponse(AsyncWebServerRequest *request, int code, const String &contentType, const String &content)
{
  request->send(code, contentType, content);
}

void sendApiResponse(AsyncWebServerRequest *request, int code)
{
  sendApiResponse(request, code, "", "");
}

void sendApiResponse(AsyncWebServerRequest *request, int code, JsonDocument doc)
{
  String response;
  serializeJson(doc, response);

  sendApiResponse(request, code, "application/json", response);
}

void sendApiResponse(AsyncWebServerRequest *request, int code, const String &message)
{
  JsonDocument doc;
  doc["message"] = message;

  sendApiResponse(request, code, doc);
}