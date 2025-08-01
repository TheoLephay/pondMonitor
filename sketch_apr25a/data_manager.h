#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "stdint.h"
#include "WString.h"


#define HOURLY_TEMP_ARRAY_SZ 24u

String DataMgr_getUptimeStr(void);

void DataMgr_pushTemp(float t);

float *DataMgr_getTempsToDisplay(uint8_t size);

void DataMgr_registerTempNotifCb(void(*cb)(void));


#endif