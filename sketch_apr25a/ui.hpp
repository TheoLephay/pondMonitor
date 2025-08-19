/// Copyright (c) 2025 Theo Lephay - All Right Reserved

#ifndef _UI_H_
#define _UI_H_

#include <Arduino.h>

#define R2_BUTTON_ON_IDX       2u
#define R2_BUTTON_OFF_IDX      3u
#define FLOAT1_INFO_CLOSED_IDX 4u
#define FLOAT1_INFO_OPEN_IDX   5u
#define FLOAT2_INFO_CLOSED_IDX 6u
#define FLOAT2_INFO_OPEN_IDX   7u
#define R3_BUTTON_ON_IDX       10u
#define R3_BUTTON_OFF_IDX      11u
#define R4_BUTTON_ON_IDX       12u
#define R4_BUTTON_OFF_IDX      13u
#define R5_BUTTON_ON_IDX       14u
#define R5_BUTTON_OFF_IDX      15u
#define CLR_ERROR_IDX          16u
#define PUMP_INFO_CLOSED_IDX   17u
#define PUMP_INFO_OPEN_IDX     18u

#define R2_BUTTON_ON_MSK       (1u << R2_BUTTON_ON_IDX)
#define R2_BUTTON_OFF_MSK      (1u << R2_BUTTON_OFF_IDX)
#define FLOAT1_INFO_CLOSED_MSK (1u << FLOAT1_INFO_CLOSED_IDX)
#define FLOAT1_INFO_OPEN_MSK   (1u << FLOAT1_INFO_OPEN_IDX)
#define FLOAT2_INFO_CLOSED_MSK (1u << FLOAT2_INFO_CLOSED_IDX)
#define FLOAT2_INFO_OPEN_MSK   (1u << FLOAT2_INFO_OPEN_IDX)
#define R3_BUTTON_ON_MSK       (1u << R3_BUTTON_ON_IDX)
#define R3_BUTTON_OFF_MSK      (1u << R3_BUTTON_OFF_IDX)
#define R4_BUTTON_ON_MSK       (1u << R4_BUTTON_ON_IDX)
#define R4_BUTTON_OFF_MSK      (1u << R4_BUTTON_OFF_IDX)
#define R5_BUTTON_ON_MSK       (1u << R5_BUTTON_ON_IDX)
#define R5_BUTTON_OFF_MSK      (1u << R5_BUTTON_OFF_IDX)
#define CLR_ERROR_MSK          (1u << CLR_ERROR_IDX)
#define PUMP_INFO_CLOSED_MSK (1u << PUMP_INFO_CLOSED_IDX)
#define PUMP_INFO_OPEN_MSK   (1u << PUMP_INFO_OPEN_IDX)

#define UPTIME_TIMER_PERIOD_US 1000000ull

void UI_setup(void);

void UiTask(void *arg);

void displayError(const char str[]);

extern TaskHandle_t UiTaskHandle;

#endif