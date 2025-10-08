#ifndef MAIN_H
#define MAIN_H

#include "esp_err.h"
#include "max30102_api.h"
#include "ds3231_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DELAY_AMOSTRAGEM 40
#define MAX_TASK 2
#define BUFFER_TEMPERATURE 6 //14
#define DS18B20_CONVERSION_DELAY_MS 750

//Estructura para el scheduler
typedef struct {
    int id;
    uint64_t periodo_ms;
    TaskHandle_t handle;
    //uint64_t ultimo_ejecucion;
} programa;

//Estructura datos
typedef struct {
    int heart_rate;
    int spo2;
    float temp_float;
    int id;
    rtc_time_t fecha;
} SensorData;

typedef struct{
    char c_heart_rate[20];
    char c_spo2[20];
    char rtc_date[20];
    char rtc_time[20];
    char c_temp_float[20];
    int id;
}CharSensorData;

typedef struct{
    char first[20];
    char second[20];
}ErrorMessage;

void save_spo2(CharSensorData sent);


#endif