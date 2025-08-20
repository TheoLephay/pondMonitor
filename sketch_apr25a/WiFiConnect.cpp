/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#include "Preferences.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "ui.hpp"
#include "WiFiConnect.h"
#include "credentials.h"

Preferences preferences;

String storedSSID;
String storedPASS;

void startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP_Config", "admin");
}

bool connectWiFi(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);

    for (uint8_t i = 0; i < 2; i++) // fails a lot on first time: retry once
    {

        WiFi.begin(ssid, password);
        if (WiFi.waitForConnectResult(5000) == WL_CONNECTED)
        {
            Serial.printf(" IP Address: %s\n", WiFi.localIP());
            return true;
        }
        WiFi.disconnect();
    }

    return false;
}


void smartWifiConnect(void)
{
    preferences.begin("wifi", false);

    String ssid = preferences.getString("ssid", "");
    String pass = preferences.getString("pass", "");

    bool connected = false;
    if (ssid.length() > 0) {
        connected = connectWiFi(ssid.c_str(), pass.c_str());
    }

    if (!connected) {
        startAP();
    }

    server.on("/newWifi", HTTP_POST, [](AsyncWebServerRequest *request) {
        if(!request->authenticate(web_id, web_password))
        {
            return request->requestAuthentication();
        }

        if (request->hasParam("ssid", true) && request->hasParam("password", true))
        {
            // Allow empty string to force reset
            String newSsid = request->getParam("ssid", true)->value();
            String newPass = request->getParam("password", true)->value();

            preferences.putString("ssid", newSsid);
            preferences.putString("pass", newPass);

            request->send(200, "text/pain", "OK. Rebooting.");

            delay(1000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "Error, post new ssid and password");
        }
    });

    server.on("/wifiForm", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!request->authenticate(web_id, web_password))
        {
            return request->requestAuthentication();
        }
        request->send(200, "text/html", WIFI_FORM_STR);
    });

    server.begin();

    if(!connected)  // Don't return in STA mode
    {
        while(1);
    }
}
