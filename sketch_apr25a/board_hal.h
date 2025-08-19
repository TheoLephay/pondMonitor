/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#ifndef _BOARD_HAL_H_
#define _BOARD_HAL_H_

#include <Arduino.h>


#define PUMP_ITR_MSK (1u << 0u)
#define FLOAT1_ITR_MSK (1u << 1u)
#define FLOAT2_ITR_MSK (1u << 2u)
#define DRUM_START_MSK (1u << 3u)
#define TEMP_MEAS_REQ_MSK (1u << 4u)
#define R3_CLOSE_MSK (1u << 5u)
#define R3_OPEN_MSK (1u << 6u)
#define R4_CLOSE_MSK (1u << 7u)
#define R4_OPEN_MSK (1u << 8u)
#define R5_CLOSE_MSK (1u << 9u)
#define R5_OPEN_MSK (1u << 10u)

#define TEMP_TIMER_PERIOD_US (1000000ull * 60ull * 60ull)
#define HB_TIMER_PERIOD_US   (1000000ull)

void Board_setup(void);

void boardTask(void *arg);

extern TaskHandle_t boardTaskHandle;

#endif