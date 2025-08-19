/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#include "PondServer.h"
#include "ui.hpp"
#include <ArduinoJson.h>
#include "ESPAsyncWebServer.h"
#include "credentials.h"
#include "data_manager.h"




void WebServer_TemperatureHander(AsyncWebServerRequest *request)
{
    if(!request->authenticate(web_id, web_password))
        return request->requestAuthentication();


    const AsyncWebParameter * param = request->getParam("number");
    if (param)
    {
        JsonDocument js;
        JsonArray array = js["temperatures"].to<JsonArray>();
        for (size_t i = 0; i < param->value().toInt(); i++)
        {
            array.add(i);
        }
        String json;
        serializeJson(js, json);
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
        request->send(response);
        return;
    }


    AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "Wrong arguments");
    request->send(response);
}


void WebServer_Setup(void)
{
    server.on("/temperature", HTTP_GET | HTTP_POST, WebServer_TemperatureHander);
}