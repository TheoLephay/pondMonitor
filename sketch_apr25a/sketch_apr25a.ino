/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include <board_hal.h>
#include <ui.hpp>
#include <esp_task_wdt.h>
#include <credentials.h>
#include "data_manager.h"
#include "WiFiConnect.h"


void setup() {
    Serial.begin(115200);

    smartWifiConnect();

    BaseType_t ret = xTaskCreate(
        boardTask,
        "Board task",
        2048,
        NULL,
        2,
        &boardTaskHandle
    );
    if (ret != pdPASS){
        Serial.println("board task creation failed");
        esp_restart();
    }

    ret = xTaskCreate(
        UiTask,
        "UI task",
        4096,
        NULL,
        3,
        &UiTaskHandle
    );
    if (ret != pdPASS){
        Serial.println("UI task creation failed");
        esp_restart();
    }

    UI_setup();
    Board_setup();
    DataMgr_Setup();

    // once both tasks have been set up, request a first temp measurement and init float statuses
    xTaskNotify(boardTaskHandle, TEMP_MEAS_REQ_MSK | PUMP_ITR_MSK | FLOAT1_ITR_MSK | FLOAT2_ITR_MSK, eSetBits);
}

void loop() {
}

