/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#ifndef WIFICONNECT_H
#define WIFICONNECT_H

void smartWifiConnect(void);

const char WIFI_FORM_STR[] =
    "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<style>"
    "body { font-family: Arial, sans-serif; padding:20px; margin:0; }"
    "form { max-width:400px; margin:auto; display:flex; flex-direction:column; gap:12px; }"
    "input { padding:12px; font-size:16px; border:1px solid #ccc; border-radius:8px; width:100%; box-sizing:border-box; }"
    "input[type=submit] { background:#007bff; color:white; border:none; cursor:pointer; }"
    "input[type=submit]:hover { background:#0056b3; }"
    "</style></head><body>"
    "<h2 style='text-align:center;'>WiFi wonfiguration</h2>"
    "<form method='POST' action='/newWifi'>"
    "<input name='ssid' placeholder='SSID'>"
    "<input name='password' type='password' placeholder='Password'>"
    "<input type='submit' value='Connect'>"
    "</form>"
    "</body></html>";

#endif