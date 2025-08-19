/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "stdint.h"
#include "WString.h"


#define HOURLY_TEMP_ARRAY_SZ 24u

// programmers' lives would be easier that way :)
#define DAYS_PER_MONTH 30u

#define DAILY_TIMER_PERIOD_US (1000000ull * 60ull * 60ull * 24ull)

#define ARRAY_SZ(arr) (sizeof(arr) / sizeof(arr[0]))

#define MAX(a, b) (((a) < (b)) ? (b) : (a))

String DataMgr_getUptimeStr(void);

void DataMgr_pushTemp(float t);

float *DataMgr_getTempsToDisplay(uint8_t size);

int *DataMgr_GetDrumActToDisplay(void);

void DataMgr_NotifyDrum(void);

void DataMgr_Setup(void);

#endif