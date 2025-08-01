#include <Arduino.h>
#include "portmacro.h"
#include "ui.hpp"
#include "data_manager.h"
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include "board_hal.h"
#include "WString.h"

#define UPDATE_TEMP_MSK (1u << 0u)
#define DISPLAY_ERROR_MSK (1u << 1u)


int hourlyTempsX[HOURLY_TEMP_ARRAY_SZ] = { 0 };
const char *statusString = NULL;

AsyncWebServer server(80);
ESPDash dashboard(&server);

Card temperature(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");

Card R2Button(&dashboard, BUTTON_CARD, "R2 - DRUM");
Card R3Button(&dashboard, BUTTON_CARD, "R3 - LIGHTS");
Card R4Button(&dashboard, BUTTON_CARD, "R4");
Card R5Button(&dashboard, BUTTON_CARD, "R5");
Card ClrButton(&dashboard, BUTTON_CARD, "Clear error");

Card pumpInfo(&dashboard, STATUS_CARD, "Pump status", DASH_STATUS_SUCCESS);
Card float1Info(&dashboard, STATUS_CARD, "Float 1 status", DASH_STATUS_SUCCESS);
Card float2Info(&dashboard, STATUS_CARD, "switch status", DASH_STATUS_SUCCESS);

Chart hourlyTempChart(&dashboard, BAR_CHART, "hourly temperatures");

Card softwareInfo(&dashboard, STATUS_CARD, "software status", DASH_STATUS_IDLE);
Card uptimeInfo(&dashboard, STATUS_CARD, "Uptime", DASH_STATUS_SUCCESS);

void updateUptime(void* pData);
esp_timer_handle_t uptimeTimerHandle = { 0 };
esp_timer_create_args_t uptimeTimerData = {
  .callback = updateUptime,
  .arg = NULL,
  .name = "uptimeTimerData",
  .skip_unhandled_events = true,
};

TaskHandle_t UiTaskHandle;

void updateUptime(void* pData)
{
    String uptime = DataMgr_getUptimeStr();
    uptimeInfo.update(uptime);
}

void updateTempCb(void)
{
    xTaskNotify(UiTaskHandle, UPDATE_TEMP_MSK, eSetValueWithOverwrite);
}

void updateTemp(void)
{
    float *hourlyTempsY = DataMgr_getTempsToDisplay(HOURLY_TEMP_ARRAY_SZ);

    hourlyTempChart.updateY(hourlyTempsY, HOURLY_TEMP_ARRAY_SZ);
    temperature.update(hourlyTempsY[0]);
}

void displayError(const char *str)
{
    statusString = str;
    xTaskNotify(UiTaskHandle, DISPLAY_ERROR_MSK, eSetValueWithOverwrite);
}

void R2ButtonCb(int value)
{
    if(value)
    {
        xTaskNotify(UiTaskHandle, R2_BUTTON_ON_MSK, eSetBits);
        xTaskNotify(boardTaskHandle, DRUM_START_MSK, eSetBits);
    }
}

void R3ButtonCb(int value)
{
    if(value)
    {
        xTaskNotify(UiTaskHandle, R3_BUTTON_ON_MSK, eSetBits);
        xTaskNotify(boardTaskHandle, R3_CLOSE_MSK, eSetBits);
    }
    else
    {
        xTaskNotify(UiTaskHandle, R3_BUTTON_OFF_MSK, eSetBits);
        xTaskNotify(boardTaskHandle, R3_OPEN_MSK, eSetBits);
    }
}

void R4ButtonCb(int value)
{
    if(value)
    {
        xTaskNotify(UiTaskHandle, R4_BUTTON_ON_MSK, eSetBits);
        xTaskNotify(boardTaskHandle, R4_CLOSE_MSK, eSetBits);
    }
    else
    {
        xTaskNotify(UiTaskHandle, R4_BUTTON_OFF_MSK, eSetBits);
        xTaskNotify(boardTaskHandle, R4_OPEN_MSK, eSetBits);
    }
}

void R5ButtonCb(int value)
{
    if(value)
    {
        xTaskNotify(UiTaskHandle, R5_BUTTON_ON_MSK, eSetBits);
        xTaskNotify(boardTaskHandle, R5_CLOSE_MSK, eSetBits);
    }
    else
    {
        xTaskNotify(UiTaskHandle, R5_BUTTON_OFF_MSK, eSetBits);
        xTaskNotify(boardTaskHandle, R5_OPEN_MSK, eSetBits);
    }
}

void ClrErrorButtonCb(int value)
{
    statusString = "All good";
    xTaskNotify(UiTaskHandle, CLR_ERROR_MSK, eSetBits);
}

void UI_setup(void)
{   
    server.begin();

    for(uint8_t i = 0; i < HOURLY_TEMP_ARRAY_SZ; i++)
    {
        hourlyTempsX[i] = -i;
    }
    hourlyTempChart.updateX(hourlyTempsX, HOURLY_TEMP_ARRAY_SZ);
    float *hourlyTempsY = DataMgr_getTempsToDisplay(HOURLY_TEMP_ARRAY_SZ);
    hourlyTempChart.updateY(hourlyTempsY, HOURLY_TEMP_ARRAY_SZ);
    DataMgr_registerTempNotifCb(updateTempCb);

    R2Button.attachCallback(R2ButtonCb);
    R3Button.attachCallback(R3ButtonCb);
    R4Button.attachCallback(R4ButtonCb);
    R5Button.attachCallback(R5ButtonCb);
    ClrButton.attachCallback(ClrErrorButtonCb);

    esp_timer_create(&uptimeTimerData, &uptimeTimerHandle);
    esp_timer_start_periodic(uptimeTimerHandle, 10 * 1000000);
    updateUptime(NULL);

    softwareInfo.update("UI initialized", DASH_STATUS_SUCCESS);
    dashboard.sendUpdates();
}

void UiTask(void *arg)
{
    uint32_t notificationFlags;
    while(1)
    {
        BaseType_t notifReceived = xTaskNotifyWait(0x0u, 0xFFFFFFFFu, &notificationFlags, pdMS_TO_TICKS(portMAX_DELAY));

        if (notifReceived == pdFAIL) continue;

        if ((notificationFlags & UPDATE_TEMP_MSK) != 0u)
        {
            updateTemp();
        }

        if ((notificationFlags & DISPLAY_ERROR_MSK) != 0u)
        {
            if(statusString != NULL)
            {
                softwareInfo.update(statusString, DASH_STATUS_DANGER);
            }
            else
            {
                softwareInfo.update("Unknown error", DASH_STATUS_DANGER);
            }
        }
        if((notificationFlags & CLR_ERROR_MSK) != 0u)
        {
            softwareInfo.update(statusString, DASH_STATUS_SUCCESS);
        }

        if ((notificationFlags & (R2_BUTTON_ON_MSK | R2_BUTTON_OFF_MSK)) != 0u)
        {
            R2Button.update((notificationFlags & R2_BUTTON_ON_MSK) != 0u);
        }
        if ((notificationFlags & (R3_BUTTON_ON_MSK | R3_BUTTON_OFF_MSK)) != 0u)
        {
            R3Button.update((notificationFlags & R3_BUTTON_ON_MSK) != 0u);
        }
        if ((notificationFlags & (R4_BUTTON_ON_MSK | R4_BUTTON_OFF_MSK)) != 0u)
        {
            R4Button.update((notificationFlags & R4_BUTTON_ON_MSK) != 0u);
        }
        if ((notificationFlags & (R5_BUTTON_ON_MSK | R5_BUTTON_OFF_MSK)) != 0u)
        {
            R5Button.update((notificationFlags & R5_BUTTON_ON_MSK) != 0u);
        }

        if ((notificationFlags & FLOAT1_INFO_CLOSED_MSK) != 0u)
        {
            float1Info.update("Closed", DASH_STATUS_DANGER);
        }
        if ((notificationFlags & FLOAT1_INFO_OPEN_MSK) != 0u)
        {
            float1Info.update("Open", DASH_STATUS_SUCCESS);
        }
        if ((notificationFlags & FLOAT2_INFO_CLOSED_MSK) != 0u)
        {
            float2Info.update("Closed", DASH_STATUS_DANGER);
        }
        if ((notificationFlags & FLOAT2_INFO_OPEN_MSK) != 0u)
        {
            float2Info.update("Open", DASH_STATUS_SUCCESS);
        }

        dashboard.sendUpdates();
    }
}


