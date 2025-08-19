/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#include "esp_timer.h"
#include <stdint.h>
#include "data_manager.h"
#include "Arduino.h"
#include "WString.h"
#include "ui.hpp"

#define MICROS_PER_MIN (1000'000ULL * 60ULL)
#define MICROS_PER_HOUR (MICROS_PER_MIN * 60ULL)
#define MICROS_PER_DAY (MICROS_PER_HOUR * 24ULL)

// Last measurement is index 0, 1h ago is index 1
static float hourlyTempsY[HOURLY_TEMP_ARRAY_SZ] = { 0.0f };

// Today is index 0, yesterday index 1
static int dailyDrumActivation[DAYS_PER_MONTH] = { 0 };

// Daily timer
void DataMgr_DailyCallback(void *arg);
esp_timer_handle_t dailyTimerHandle = { 0 };
esp_timer_create_args_t dailyTimerData = {
  .callback = DataMgr_DailyCallback,
  .arg = NULL,
  .name = "DataMgr Daily",
  .skip_unhandled_events = true,
};


///-----------------------
/// Temperature management
///-----------------------

void (*DataMgr_notifyNewTemp)(void);


float *DataMgr_getTempsToDisplay(uint8_t size)
{
    return hourlyTempsY;
}


template<typename T>
static void DataMgr_shiftAll(T *array, uint32_t size)
{
    for (uint32_t i = size - 1; i > 0; i--) {
        array[i] = array[i - 1];
    }
}

void DataMgr_pushTemp(float t)
{
    DataMgr_shiftAll(hourlyTempsY, ARRAY_SZ(hourlyTempsY));

    hourlyTempsY[0] = t;

    xTaskNotify(UiTaskHandle, UPDATE_TEMP_MSK, eSetBits);
}


///------------------
/// UpTime management
///------------------

String DataMgr_getUptimeStr(void)
{
    uint64_t micros = esp_timer_get_time();

    uint16_t days = micros / MICROS_PER_DAY;
    micros %= MICROS_PER_DAY;

    uint8_t hours = micros / MICROS_PER_HOUR;
    micros %= MICROS_PER_HOUR;

    uint8_t mins = micros / MICROS_PER_MIN;
    micros %= MICROS_PER_MIN;

    uint8_t secs = micros / 1000'000ULL;

    return String(days) + "d" + String(hours) + "h" + String(mins) + "m" + String(secs) + "s";
}


///---------------------------
/// Drum activation management
///---------------------------

int *DataMgr_GetDrumActToDisplay(void)
{
    return dailyDrumActivation;
}

void DataMgr_NotifyDrum(void)
{
    dailyDrumActivation[0u]++;
    xTaskNotify(UiTaskHandle, UPDATE_DRUM_MSK, eSetBits);
}

void DataMgr_DailyCallback(void *arg)
{
    // shift drum data by one day
    DataMgr_shiftAll(dailyDrumActivation, ARRAY_SZ(dailyDrumActivation));
    dailyDrumActivation[0u] = 0;
    xTaskNotify(UiTaskHandle, UPDATE_DRUM_MSK, eSetBits);
}

void DataMgr_Setup(void)
{
    esp_timer_create(&dailyTimerData, &dailyTimerHandle);
    esp_timer_start_periodic(dailyTimerHandle, DAILY_TIMER_PERIOD_US);
}

