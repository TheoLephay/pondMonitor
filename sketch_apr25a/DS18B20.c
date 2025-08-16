/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#include "esp_timer.h"
#include <stdint.h>
#include "DS18B20.h"
#include "Arduino.h"


static void DS_Delay(uint32_t us)
{
    uint32_t start = esp_timer_get_time();
    while ((uint32_t) esp_timer_get_time() - start < us);
}


static void DS_LowPulse(uint32_t us)
{
    gpio_set_level(TEMP_PIN, 0u);
    DS_Delay(us);
    gpio_set_level(TEMP_PIN, 1u);
}


static bool DS_GetLevel()
{
    return gpio_get_level(TEMP_PIN) == 1u;
}


static void DS_WriteBit(bool b)
{
    vTaskSuspendAll();
    if(b)
    {
        DS_LowPulse(T_LOW_1);
        DS_Delay(T_LOW_0 - T_LOW_1);
    }
    else
    {
        DS_LowPulse(T_LOW_0);
    }
    DS_Delay(T_REC);
    xTaskResumeAll();
}


static bool DS_ReadBit(void)
{
    vTaskSuspendAll();
    DS_LowPulse(T_LOW_1);
    DS_Delay(T_SAMPLE - T_LOW_1);
    bool val = DS_GetLevel();
    xTaskResumeAll();

    DS_Delay(T_SLOT - T_SAMPLE);
    return val;
}


static void DS_WriteByte(uint8_t byte)
{
    for(uint8_t b = 0; b < 8; b++)
    {
        DS_WriteBit(byte & 1u);
        byte >>= 1;
    }
}


static uint8_t DS_ReadByte(void)
{
    uint8_t byte = 0;
    for(uint8_t b = 0; b < 8; b++)
    {
        byte |= (DS_ReadBit() << b);
    }
    return byte;
}


static bool DS_Waitlevel(bool level, uint32_t timeout, uint32_t *duration)
{
    const uint32_t start = esp_timer_get_time();
    uint32_t end = start;
    bool error = false;
    while(!error && (DS_GetLevel() == level))
    {
        end = (uint32_t) esp_timer_get_time();
        if (end - start >= timeout)
        {
            error = true;
        }
    }
    if(duration != NULL)
    {
        *duration = end - start;
    }
    return error;
}


uint64_t DS_ReadSingleRom(void)
{
    const uint8_t opcode = 0x33;
    DS_WriteByte(opcode);

    uint64_t response = 0;

    for(uint8_t byte_idx = 0; byte_idx < 8; byte_idx++)
    {
        response |= ((uint64_t) DS_ReadByte() << (8u * byte_idx));
    }

    return response;
}


void DS_MatchRom(uint64_t rom)
{
    const uint8_t opcode = 0x55;
    DS_WriteByte(opcode);
    for(uint8_t byte_idx = 0; byte_idx < 8; byte_idx++)
    {
        DS_WriteByte(((rom >> (byte_idx * 8u)) & 0xFF));
    }
}


void DS_SkipRom(void)
{
    const uint8_t opcode = 0xCC;
    DS_WriteByte(opcode);
}


bool DS_CheckPresence(void)
{
    uint32_t duration;

    DS_LowPulse(T_RSTL);

    vTaskSuspendAll();

    if (DS_Waitlevel(1u, RESP_TIMEOUT, NULL))
    {
        xTaskResumeAll();
        return false;
    }
    if (DS_Waitlevel(0u, T_RST_MAX, &duration))
    {
        xTaskResumeAll();
        return false;
    }

    xTaskResumeAll();

    DS_Delay(T_REC);

    return (duration > T_RST_MIN) && (duration < T_RST_MAX);
}


float DS_ReadTemp(void)
{
    const uint8_t opcode = 0xBE;
    DS_WriteByte(opcode);

    uint16_t raw = 0;
    raw |= DS_ReadByte();
    raw |= (uint16_t) DS_ReadByte() << 8u;

    float resp = (float) (int16_t) raw;

    return resp / 16.0f;
}


void DS_StartConversion(void)
{
    const uint8_t opcode = 0x44;
    DS_WriteByte(opcode);
}


void DS_WaitConverion(void)
{
    while(DS_ReadBit() == 0u);
}


void DS_StartAndWaitConversion(void)
{
    DS_StartConversion();
    DS_WaitConverion();
}


void DS_Setup(void)
{
    gpio_set_level(TEMP_PIN, 1);
    gpio_set_direction(TEMP_PIN, GPIO_MODE_INPUT_OUTPUT_OD);
}
