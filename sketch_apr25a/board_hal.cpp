#include <Arduino.h>
#include "esp32-hal.h"
#include "esp32-hal-gpio.h"
#include "DHT22.h"
#include <board_hal.h>
#include "esp_timer.h"
#include "data_manager.h"
#include "ui.hpp"


#define HB_LED     2u
#define TEMP_SENSOR_PIN 16u
#define DEBOUNCER_POLLS 31u

#define TEMP_SENSOR_TRIALS 2u
#define TEMP_SENSOR_PERIOD_US 60UUL * 60UUL * 1000000ULL
#define DRUM_DURATION_MS 5000u

#define RELAY2_PIN  23
#define RELAY3_PIN  22
#define RELAY4_PIN  21
#define RELAY5_PIN  19

#define PUMP_PIN    34
#define FLOAT1_PIN  32
#define FLOAT2_PIN  35

TaskHandle_t boardTaskHandle;

bool r3State = false;

void heartBeatCB(void *arg);
void TempMeasureCb(void *args);
void boardTask(void *arg);

typedef struct {
    const uint8_t pin;
    uint32_t counter;
} WaterFloat_t;

WaterFloat_t pump = {.pin = PUMP_PIN, .counter = 0};
WaterFloat_t float1 = {.pin = FLOAT1_PIN, .counter = 0};
WaterFloat_t float2 = {.pin = FLOAT2_PIN, .counter = 0};

DHT22 dht22(TEMP_SENSOR_PIN);

bool tempMeasReq = false;
uint64_t drumStartTime = 0;
bool drumRunning = false;

esp_timer_handle_t tempTimerHandle = { 0 };
esp_timer_create_args_t tempTimerData = {
  .callback = TempMeasureCb,
  .arg = NULL,
  .name = "tempTimerData",
  .skip_unhandled_events = true,
};

esp_timer_handle_t heartBeatTimerHandle = { 0 };
esp_timer_create_args_t heartBeatTimerData = {
  .callback = heartBeatCB,
  .arg = NULL,
  .name = "heatBeat",
  .skip_unhandled_events = true,
};

static portMUX_TYPE tempSpinlock = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR inputGpioCb(void *arg)
{
    WaterFloat_t itrFloat = *(WaterFloat_t *)arg;
    BaseType_t HigherPriorityTaskWoken = pdFALSE;
    switch (itrFloat.pin) {
        case PUMP_PIN:
            xTaskNotifyFromISR(boardTaskHandle, PUMP_ITR_MSK, eSetBits, &HigherPriorityTaskWoken);
            break;
        case FLOAT1_PIN:
            xTaskNotifyFromISR(boardTaskHandle, FLOAT1_ITR_MSK, eSetBits, &HigherPriorityTaskWoken);
            break;
        case FLOAT2_PIN:
            xTaskNotifyFromISR(boardTaskHandle, FLOAT2_ITR_MSK, eSetBits, &HigherPriorityTaskWoken);
            break;
        default:
            break;
    }
    portYIELD_FROM_ISR(HigherPriorityTaskWoken);
}


void heartBeatCB(void *arg)
{
    static bool hb = false;
    digitalWrite(HB_LED, hb);
    hb = !hb;
}

void TempMeasureCb(void *args)
{
    xTaskNotify(boardTaskHandle, TEMP_MEAS_REQ_MSK, eSetBits);
}

void TempMeasure(void)
{
    unsigned int trials = TEMP_SENSOR_TRIALS;
    float t;
    do
    {
        if (trials < TEMP_SENSOR_TRIALS) vTaskDelay(2000);
        taskENTER_CRITICAL(&tempSpinlock);
        t = dht22.getTemperature();
        taskEXIT_CRITICAL(&tempSpinlock);
    }
    while ((dht22.getLastError() != dht22.OK) && (--trials > 0u));


    if (dht22.getLastError() != dht22.OK)
    {
        Serial.printf("dht error %u\n", dht22.getLastError());
        displayError("ERR Comm temp sensor");
    }
    else
    {
        DataMgr_pushTemp(t);
    }
}

static bool Board_digitalReadDebounced(uint8_t pin)
{
    int posCnt = 0;
    for(int i = 0; i < DEBOUNCER_POLLS; i++)
    {
        posCnt += digitalRead(pin);
    }
    return posCnt > (DEBOUNCER_POLLS / 2);
}

static void Board_startDrum(void)
{
    if (drumRunning) return;

    drumRunning = true;
    drumStartTime = millis();
    digitalWrite(RELAY2_PIN, 1);
}

static void Board_checkStopDrum(void)
{
    if (!drumRunning) return;

    if(millis() - drumStartTime > DRUM_DURATION_MS)
    {
        drumRunning = false;
        digitalWrite(RELAY2_PIN, 0);
        xTaskNotify(UiTaskHandle, R2_BUTTON_OFF_MSK, eSetBits);
    }
}


void Board_setup(void)
{
    esp_timer_create(&tempTimerData, &tempTimerHandle);
    esp_timer_start_periodic(tempTimerHandle, 60 * 30 * 1000000);

    esp_timer_create(&heartBeatTimerData, &heartBeatTimerHandle);
    esp_timer_start_periodic(heartBeatTimerHandle, 1000000);
    pinMode(HB_LED, OUTPUT);
    digitalWrite(HB_LED, 1);


    pinMode(pump.pin,   INPUT_PULLUP);
    pinMode(float1.pin, INPUT);
    pinMode(float2.pin, INPUT);
    attachInterruptArg(pump.pin,   inputGpioCb, (void *) &pump,   CHANGE);
    attachInterruptArg(float1.pin, inputGpioCb, (void *) &float1, CHANGE);
    attachInterruptArg(float2.pin, inputGpioCb, (void *) &float2, CHANGE);

    pinMode(RELAY2_PIN, OUTPUT);
    pinMode(RELAY3_PIN, OUTPUT);
    pinMode(RELAY4_PIN, OUTPUT);
    pinMode(RELAY5_PIN, OUTPUT);
}



void boardTask(void *arg)
{
    uint32_t notificationFlags;
    bool pinValue;
    while(1)
    {
        BaseType_t notifReceived = xTaskNotifyWait(0x0u, 0xFFFFFFFFu, &notificationFlags, pdMS_TO_TICKS(500));

        if ((notificationFlags & PUMP_ITR_MSK) != 0u)
        {
            // TODO if too close, maybe skip...
            // OTOD 2: ajouter compteur de temps
        }

        if ((notificationFlags & FLOAT1_ITR_MSK) != 0u)
        {
            pinValue = Board_digitalReadDebounced(FLOAT1_PIN);
            if (pinValue)
            {
                xTaskNotify(boardTaskHandle, DRUM_START_MSK, eSetBits);
                xTaskNotify(UiTaskHandle, R2_BUTTON_ON_MSK | FLOAT1_INFO_CLOSED_MSK, eSetBits);
            }
            else
            {
                xTaskNotify(UiTaskHandle, FLOAT1_INFO_OPEN_MSK, eSetBits);
            }
        }

        if ((notificationFlags & FLOAT2_ITR_MSK) != 0u)
        {
            pinValue = Board_digitalReadDebounced(FLOAT2_PIN);
            if (pinValue)
            {
                xTaskNotify(UiTaskHandle, FLOAT2_INFO_CLOSED_MSK, eSetBits);
            }
            else
            {
                xTaskNotify(UiTaskHandle, FLOAT2_INFO_OPEN_MSK, eSetBits);
            }
            if (!r3State)
            {
                xTaskNotify(boardTaskHandle, R3_CLOSE_MSK, eSetBits);
                xTaskNotify(UiTaskHandle, R3_BUTTON_ON_MSK, eSetBits);
            }
            else
            {
                xTaskNotify(boardTaskHandle, R3_OPEN_MSK, eSetBits);
                xTaskNotify(UiTaskHandle, R3_BUTTON_OFF_MSK, eSetBits);
            }
        }

        if ((notificationFlags & R3_CLOSE_MSK) != 0u)
        {
            digitalWrite(RELAY3_PIN, 1);
            r3State = true;
        }
        if ((notificationFlags & R3_OPEN_MSK) != 0u)
        {
            digitalWrite(RELAY3_PIN, 0);
            r3State = false;
        }
        if ((notificationFlags & R4_CLOSE_MSK) != 0u)
        {
            digitalWrite(RELAY4_PIN, 1);
        }
        if ((notificationFlags & R5_OPEN_MSK) != 0u)
        {
            digitalWrite(RELAY4_PIN, 0);
        }
        if ((notificationFlags & R5_CLOSE_MSK) != 0u)
        {
            digitalWrite(RELAY5_PIN, 1);
        }
        if ((notificationFlags & R5_OPEN_MSK) != 0u)
        {
            digitalWrite(RELAY5_PIN, 0);
        }

        if ((notificationFlags & TEMP_MEAS_REQ_MSK) != 0u)
        {
            TempMeasure();
        }

        if ((notificationFlags & DRUM_START_MSK) != 0u)
        {
            Board_startDrum();
        }

        // drum routine with wait timeout
        Board_checkStopDrum();
    }
}


