#include "esp_timer.h"
#include <stdint.h>
#include "data_manager.h"
#include "Arduino.h"
#include "WString.h"

#define MICROS_PER_MIN (1000'000ULL * 60ULL)
#define MICROS_PER_HOUR (MICROS_PER_MIN * 60ULL)
#define MICROS_PER_DAY (MICROS_PER_HOUR * 24ULL)

float hourlyTempsY[HOURLY_TEMP_ARRAY_SZ] = { 0 };



///-----------------------
/// Temperature management
///-----------------------

void (*DataMgr_notifyNewTemp)(void);


float *DataMgr_getTempsToDisplay(uint8_t size)
{
    return hourlyTempsY;
}

static void DataMgr_shiftAll(void)
{
    for (uint8_t i = HOURLY_TEMP_ARRAY_SZ - 1; i > 0; i--) {
        hourlyTempsY[i] = hourlyTempsY[i - 1];
    }
}


void DataMgr_pushTemp(float t)
{
    DataMgr_shiftAll();

    hourlyTempsY[0] = t;
    
    if(DataMgr_notifyNewTemp != NULL)
    {
        DataMgr_notifyNewTemp();
    }
}


void DataMgr_registerTempNotifCb(void(*cb)(void))
{
    DataMgr_notifyNewTemp = cb;
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
