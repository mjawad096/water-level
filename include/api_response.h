#include <ArduinoJson.h>

#pragma once

String apiResponse(String message)
{
  JsonDocument doc;
  doc["message"] = message;
  String response;
  serializeJson(doc, response);
  return response;
}