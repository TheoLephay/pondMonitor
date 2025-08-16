/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#ifndef DS18B20_H
#define DS18B20_H

#include "stdbool.h"
#include "stdint.h"

// All values below come from the datasheet and are un microseconds if not specified
#define TEMP_PIN 16u

// min 480us
#define T_RSTL 480u

#define T_RST_MIN 60u
#define T_RST_MAX 240u

// Real T_pdhigh - 5us
#define T_PDHIGH_MIN 10u

// 60 - 120 us
#define T_LOW_0 70u

// 1 - 15 us
#define T_LOW_1 3u

// lower than data validity period (15us)
#define T_SAMPLE 10u

// Read opration: min 60us
#define T_SLOT 60u

// recovery between two bits, > 1us
#define T_REC 2u

#define RESP_TIMEOUT 60u

// in milliseconds
// If you have the original DS18B20 from Analog Device, set this to 750ms.
// In my case I have a chinese copy that takes ~30ms to do a conversion.
#define CONVERSION_TIME 35u

// Returns presence
bool DS_CheckPresence(void);

void DS_Setup(void);

uint64_t DS_ReadSingleRom(void);

float DS_ReadTemp(void);

void DS_StartConversion(void);

void DS_StartAndWaitConversion(void);

void DS_SkipRom(void);

void DS_MatchRom(uint64_t rom);

// Returns success
static inline bool DS_StartConversionUniqueSensor(void)
{
    if (!DS_CheckPresence())
    {
        return false;
    }
    DS_SkipRom();
    DS_StartConversion();
    return true;
}

static inline float DS_GetTempUniqueSensor(void)
{
    DS_CheckPresence();
    DS_SkipRom();
    return DS_ReadTemp();
}

#endif // DS18B20_H