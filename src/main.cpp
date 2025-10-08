#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include "esp_log.h"
#include <esp_check.h>
#include "main.h"
#include "i2c_api.h"
#include "max30102_api.h"
#include <string.h>
#include "ssd1306.h"
#include <inttypes.h>
#include "ds3231_api.h" 
#include "algorithm.h"
#include "ssd_oled.h" 
#include "../lib/libsd/SDManager.h"
//#include "config_max.h"

TaskHandle_t sensor_reader_handle = NULL;
SemaphoreHandle_t sem_spo2_done;
SemaphoreHandle_t sem_temp_done;
QueueHandle_t sensor_data_queue;
QueueHandle_t oled_data_queue;
ssd1306_handle_t dev_hdl = NULL;   // se llena en i2c0_init
i2c_master_dev_handle_t ds3231_handle = NULL;

//Variables para la SD
bool is_sdcard_enabled=false;
static const char *TAGSD = "SD-CARD";
bool bandera = false;
bool modo = false;
static sdmmc_card_t *sdcard;
#define MOUNT_POINT "/sdcard"
#define SD_SUPPORTED

SDManager sdm;

// Variables globales
static const char *TAG = "SPI-SLAVE";
programa task[MAX_TASK];

int32_t red_data_buffer[BUFFER_SIZE];
int32_t ir_data_buffer[BUFFER_SIZE];
int32_t red_snorm[BUFFER_SIZE];
int32_t ir_snorm[BUFFER_SIZE];
float red_norm[BUFFER_SIZE];
float red_norm2[16];
int32_t nuevo[16];

int32_t red_data = 0;
int32_t ir_data = 0;
int32_t min = 0;
int32_t max = 0;
int32_t min2 = 0;
int32_t max2 = 0;
float finger_on = 0;

float dec_temp[BUFFER_TEMPERATURE];
int en_temp[BUFFER_TEMPERATURE];

extern "C" bool verificar(void);
extern "C" bool detect_sensor(void);
extern "C" void one_wire_write_byte(uint8_t);
extern "C" uint8_t one_wire_read_byte(void);


// Contantes de tiempo
#define DELAY_BEFORE_START_MS 30000 // 30 segundos
#define SENSOR_HR_RUN_TIME_MS  102240// 25.560 segundos *4 = 102240 ms
#define SENSOR_TEMP_RUN_TIME_MS 4500  // 10.5 segundos

extern "C" bool verificar(void);

extern "C" void save_SD(CharSensorData sent){
    
    #ifdef SD_SUPPORTED
        if (sdm.init_sdspi(sdcard, MOUNT_POINT) == ESP_OK) {             
            is_sdcard_enabled = true;
            ESP_LOGI(TAG, "SD montada correctamente");
            sdm.setSDcard(true);
        } else {
            ESP_LOGE(TAG, "Error al montar la SD");
        }
    #endif


    if(sent.id == 1){
        const char *filesd = MOUNT_POINT "/spo2_fc.txt";
        printf("[guardar_datos] HR: %s SpO2: %s\n", sent.c_heart_rate, sent.c_spo2);
        if (filesd != NULL && is_sdcard_enabled){
            char data[50];
            strcat(data, sent.c_heart_rate);
            strcat(data, " - ");
            strcat(data, sent.c_spo2);
            strcat(data, " - ");
            strcat(data, sent.rtc_date);
            strcat(data, "  ");
            strcat(data, sent.rtc_time);
            strcat(data, "\n");
            esp_err_t res = sdm.writeFileDVG(filesd, data);
            if (res == ESP_OK) {
                ESP_LOGI(TAGSD, "Escrito en %s: %s", filesd, data);}
                else {
                ESP_LOGE(TAGSD, "Error escribiendo en %s", data);
                }
        }
        else{
            ESP_LOGW(TAGSD, "SD no montada, no se puede escribir");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
    else if(sent.id == 2){
        const char *filesd = MOUNT_POINT "/temp.txt";
        printf("[guardar_datos] Temp: %s\n", sent.c_temp_float);
        if (filesd != NULL && is_sdcard_enabled){
            char data[50];
            strcat(data, sent.c_temp_float);
            strcat(data, " - ");
            strcat(data, sent.rtc_date);
            strcat(data, "  ");
            strcat(data, sent.rtc_time);
            strcat(data, "\n");
            esp_err_t res = sdm.writeFileDVG(filesd, data);
            if (res == ESP_OK) {
                ESP_LOGI(TAGSD, "Escrito en %s: %s", filesd, data);}
                else {
                ESP_LOGE(TAGSD, "Error escribiendo en %s", data);
                }
        }
        else{
            ESP_LOGW(TAGSD, "SD no montada, no se puede escribir");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

// -----------------------------------------------------------------------------
// Tarea de lectura del SpO2
// -----------------------------------------------------------------------------
static void sensor_SpO2(void *arg) {
    ESP_LOGI("sensor_SpO2", "Esperando notificación para iniciar");
    TickType_t duracion;
    message_HR(dev_hdl);

    while (1) {

        TickType_t startTick = xTaskGetTickCount();
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ESP_LOGI("sensor_SpO2", "Iniciando ejecución");
        message_HR(dev_hdl);

        do{
            verificar();

            TickType_t now = xTaskGetTickCount();
            if ((now - startTick) >= pdMS_TO_TICKS(SENSOR_HR_RUN_TIME_MS)) {
                break;
            }

            // Limpiar buffers
            memset(red_data_buffer, 0, sizeof(red_data_buffer));
            memset(ir_data_buffer, 0, sizeof(ir_data_buffer));
            memset(red_snorm, 0, sizeof(red_snorm));
            memset(ir_snorm, 0, sizeof(ir_snorm));
            memset(red_norm, 0, sizeof(red_norm));
            memset(red_norm2, 0, sizeof(red_norm2));
            memset(nuevo, 0, sizeof(nuevo));

            // Inicializar y leer MAX30102
            fill_buffers_data();
            std::cout << "Se llenaron buffers\n";

            finger_on = calculate_average(ir_data_buffer, BUFFER_SIZE);

            if(finger_on < 2000) {
                std::cout << "No se detectó dedo\n";
                errorMessage(dev_hdl);
                duracion = 102240;
            }
            else{
                
                //Asegurar que el primer dato no sea muy alto
                ir_data_buffer[0] = ir_data_buffer[1];
                red_data_buffer[0] = red_data_buffer[1];

                // Copiar datos sin filtrar
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    ir_snorm[i] = ir_data_buffer[i];
                    red_snorm[i] = red_data_buffer[i];
                }

                for (int j = 0; j < 16; j++) {
                    nuevo[j] = crearArreglo(j, red_data_buffer);
                }

                // Filtrado FC
                moving_average_filter(ir_snorm, red_snorm);
                //moving_average_muestreo(nuevo, 32);
                filtroPasaBajasPrimerOrdenmuestreo(nuevo);
                
                // Filtrado SpO2
                filtroPasaBajasPrimerOrden(ir_data_buffer, red_data_buffer);
                moving_average_filter(ir_data_buffer, red_data_buffer);

                double spo2 = spo2_measurement(ir_data_buffer, red_data_buffer);

                if(spo2>100){
                    spo2 = 100;
                }

                min = calcular_min(red_snorm, BUFFER_SIZE);
                max = calcular_max(red_snorm, BUFFER_SIZE);
                min2 = calcular_min(nuevo, 16); //32
                max2 = calcular_max(nuevo, 16); //32

                for (int a = 0; a < BUFFER_SIZE; a++) {
                    red_norm[a] = normalizar(red_snorm, a, min, max);
                //std::cout << red_norm[a] << "\n";
            }

                //std::cout << "\nMuestreo\n";
                for (int a = 0; a < 16; a++) {
                    red_norm2[a] = normalizar(nuevo, a, min2, max2);
                //std::cout << red_norm2[a] << "\n";
            }

                int heart_rate = calcular_heart_rate(red_norm, BUFFER_SIZE);
                int heart_rate2 = calcular_heart_rate(red_norm2, 16);

                rtc_time_t ahora;
                if (ds3231_get_time(&ahora) == ESP_OK) {
                    ESP_LOGI("DS3231", "Hora: %02d:%02d:%02d Fecha: %02d/%02d/%d",
                    ahora.hour, ahora.min, ahora.sec,
                    ahora.date, ahora.month, ahora.year);
                }

                int heart_total = 0;
                if (heart_rate != 0 && heart_rate2 != 0)
                    heart_total = (heart_rate + heart_rate2) / 2;
                else if (heart_rate == 0)
                    heart_total = heart_rate2;
                else
                    heart_total = heart_rate;

                std::cout << "[SENSOR_SPO2] HR: " << heart_rate << " |Muestreo: " << heart_rate2 << "\n";
                std::cout << "\n[SENSOR_SPO2] HR total: " << heart_total << " | SpO2: " << spo2 << "\n";

                SensorData data;
                data.heart_rate = heart_total;
                data.spo2 = spo2;
                data.id = 1;
                data.fecha = ahora;
                xQueueSend(sensor_data_queue, &data, portMAX_DELAY);

                vTaskDelay(2000/portTICK_PERIOD_MS); //Retraso para evitar saturación de datos

                duracion = xTaskGetTickCount() - startTick;
                std::cout<< "\n[SENSOR_TEMP] Fin de ejecución. Duración total: " << pdTICKS_TO_MS(duracion) << " ms\n";
            }
        }while(duracion<pdMS_TO_TICKS(SENSOR_HR_RUN_TIME_MS));

        xSemaphoreGive(sem_spo2_done);
        ESP_LOGI("sensor_SpO2", "Ejecución completada");
    }
}

static void sensor_temp(void *arg) {
    ESP_LOGI("sensor_temp", "Esperando notificación para iniciar");

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ESP_LOGI("sensor_temp", "Inicio de ejecución");
        message_temp(dev_hdl);

        TickType_t startTick = xTaskGetTickCount();
        TickType_t currentTick;

        SensorData data;
        memset(dec_temp, 0, sizeof(dec_temp));
        memset(en_temp, 0, sizeof(en_temp));
        
        rtc_time_t ahora;
        if (ds3231_get_time(&ahora) == ESP_OK) {
             ESP_LOGI("DS3231", "Hora: %02d:%02d:%02d Fecha: %02d/%02d/%d",
             ahora.hour, ahora.min, ahora.sec,
             ahora.date, ahora.month, ahora.year);
            }

        // Antes de iniciar la toma de datos, asegurarnos que no haya expirado
        currentTick = xTaskGetTickCount();
        if ((currentTick - startTick) >= pdMS_TO_TICKS(SENSOR_TEMP_RUN_TIME_MS)) {
            ESP_LOGW("sensor_temp", "Tiempo ya expirado antes de iniciar");
            xSemaphoreGive(sem_temp_done);
            continue;
        }

        for (int i = 0; i < 6; i++) {
            if (detect_sensor()) {
                one_wire_write_byte(0xCC); // Skip ROM
                one_wire_write_byte(0x44); // Convertir temperatura
                vTaskDelay(pdMS_TO_TICKS(DS18B20_CONVERSION_DELAY_MS));

                if (detect_sensor()) {
                    one_wire_write_byte(0xCC); // Skip ROM
                    one_wire_write_byte(0xBE); // Leer datos
                    uint8_t tempLSB = one_wire_read_byte();
                    uint8_t tempMSB = one_wire_read_byte();
                    int16_t temp_raw = (tempMSB << 8) | tempLSB;

                    en_temp[i] = temp_raw / 16;
                    dec_temp[i] = (temp_raw % 16) * 625;

                    std::cout << "Temp [" << i << "]: " << en_temp[i] << "." << dec_temp[i] << "\n";
                } else {
                    std::cout << "Sensor no respondió tras conversión.\n";
                }
            } else {
                std::cout << "Sensor no detectado.\n";
            }

            // Checar si ya se excedió el tiempo entre lecturas
            currentTick = xTaskGetTickCount();
            if ((currentTick - startTick) >= pdMS_TO_TICKS(SENSOR_TEMP_RUN_TIME_MS)) {
                ESP_LOGW("sensor_temp", "Tiempo de lectura expirado en la iteración %d", i);
                break;
            }
        }

        // Guardar última temperatura 
        float dec_temp_media = dec_temp[BUFFER_TEMPERATURE - 1] / 10000.0f;
        data.temp_float = en_temp[BUFFER_TEMPERATURE - 1] + dec_temp_media;
        data.id = 2;
        data.fecha = ahora;

        xQueueSend(sensor_data_queue, &data, portMAX_DELAY);

        //TickType_t duracion = xTaskGetTickCount() - startTick;
        //std::cout<< "\n[SENSOR_TEMP] Fin de ejecución. Duración total: " << pdTICKS_TO_MS(duracion) << " ms\n";
        xSemaphoreGive(sem_temp_done);
    }
}

// -----------------------------------------------------------------------------
// Tarea controladora de ciclos
// -----------------------------------------------------------------------------
static void cycle_controller_task(void *arg) {
    ESP_LOGI("cycle_controller", "Esperando 2 minutos antes de iniciar sensores");
    Init_Message(dev_hdl);
    vTaskDelay(pdMS_TO_TICKS(DELAY_BEFORE_START_MS));
    ESP_LOGI("cycle_controller", "Iniciando ejecución cíclica de sensores");

    while (1) {

        // Notificar la tarea sensor_SpO2 para ejecutarse
        ESP_LOGI("cycle_controller", "Notificando sensor_SpO2");
        xTaskNotifyGive(task[1].handle);

        if (xSemaphoreTake(sem_spo2_done, pdMS_TO_TICKS(SENSOR_HR_RUN_TIME_MS + 2000)) == pdTRUE) {
            ESP_LOGI("cycle_controller", "sensor_spo2 completado correctamente");
        } else {ESP_LOGW("cycle_controller", "sensor_spo2 no respondió a tiempo");}
        vTaskDelay(1000);

        // Notificar la tarea sensor_temp para ejecutarse
        ESP_LOGI("cycle_controller", "Notifying sensor_temp to run");
        xTaskNotifyGive(task[0].handle); 

        if (xSemaphoreTake(sem_temp_done, pdMS_TO_TICKS(SENSOR_TEMP_RUN_TIME_MS + 2000)) == pdTRUE)
        {ESP_LOGI("cycle_controller", "sensor_temp completado correctamente");}
        else{ESP_LOGI("cycle controller", "sensor_temp no respondió a tiempo.");}
        vTaskDelay(1000);
    }
}

static void procesar_datos(void *arg)
{
    SensorData received;
    CharSensorData sent;

    while (1)
    {
        vTaskDelay(1000);
        if (xQueueReceive(sensor_data_queue, &received, portMAX_DELAY))
        {
            if(received.id == 1)
            {std::cout << "[procesar_datos] HR: " << received.heart_rate << "  SpO2: " << received.spo2 << "\n";
            snprintf(sent.c_heart_rate, sizeof(sent.c_heart_rate), "%d", received.heart_rate);
            snprintf(sent.c_spo2, sizeof(sent.c_spo2), "%d", received.spo2);
            snprintf(sent.rtc_date, sizeof(sent.rtc_date),"%02u/%02u/%04u",
            (unsigned int)received.fecha.date,(unsigned int)received.fecha.month,(unsigned int)received.fecha.year);
            snprintf(sent.rtc_time, sizeof(sent.rtc_time),"%02u:%02u:%02u",
            (unsigned int)received.fecha.hour,(unsigned int)received.fecha.min,(unsigned int)received.fecha.sec);
            sent.id = received.id;
            xQueueSend(oled_data_queue, &sent, portMAX_DELAY);
        }
            else if (received.id == 2)
            {std::cout << "[procesar_datos] Temp float: " << received.temp_float << "\n";
            snprintf(sent.c_temp_float, sizeof(sent.c_temp_float), "%2.2f", received.temp_float);
            snprintf(sent.rtc_date, sizeof(sent.rtc_date),"%02u/%02u/%04u",
            (unsigned int)received.fecha.date,(unsigned int)received.fecha.month,(unsigned int)received.fecha.year);
            snprintf(sent.rtc_time, sizeof(sent.rtc_time),"%02u:%02u:%02u",
            (unsigned int)received.fecha.hour,(unsigned int)received.fecha.min,(unsigned int)received.fecha.sec);
            sent.id = received.id;
            xQueueSend(oled_data_queue, &sent, portMAX_DELAY);
            };
        } 
        else
        {std::cout << "[procesar_datos] No se recibieron datos.\n";};
        //vTaskDelay(1000);
    }
}

static void desplegar_datos(void *arg)
{
    CharSensorData received;
    while (1)
    {
        vTaskDelay(1000);
        if (xQueueReceive(oled_data_queue, &received, portMAX_DELAY))
        {
            if(received.id == 1)
            {std::cout << "[desplegar_datos] HR: " << received.c_heart_rate << "  SpO2: " << received.c_spo2 << "\n";
            std::cout <<"[desplegar_datos] Hora:" << received.rtc_time <<"\n"; 
            std::cout <<"[desplegar_datos] Fecha:" <<received.rtc_date<< "\n";
            display_max30102(dev_hdl, received);
            save_SD(received);
            }
            else if (received.id == 2)
            {std::cout << "[desplegar_datos] Temp float: " << received.c_temp_float << "\n";
            std::cout <<"[desplegar_datos] Hora:" << received.rtc_time <<"\n";
            std::cout <<"[desplegar_datos] Fecha:" <<received.rtc_date<< "\n";
            display_DS18B20(dev_hdl, received);
            save_SD(received);
            };
        } 
        else
        {std::cout << "[procesar_datos] No se recibieron datos.\n";};
        //vTaskDelay(1000);
    }
}


extern "C" void app_main(void) {
    ESP_LOGI("app_main", "Inicializando sistema...");

    esp_err_t res_0 =i2c_creation();
    if(res_0 != ESP_OK){
        ESP_LOGE("app_main", "Error al inicializar i2c");
    }
    
    esp_err_t res2 = i2c0_init(); // SSD1306
    if (res2 != ESP_OK) {
        ESP_LOGE("app_main", "Error al inicializar SSD1306");
    }
    vTaskDelay(pdMS_TO_TICKS(50));
    esp_err_t res = i2c_init();   // MAX30102
    if (res != ESP_OK) {
        ESP_LOGE("app_main", "Error al inicializar MAX30102");
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    esp_err_t res_3 = i2c_init_ds3231();   // DS3231
    if (res_3 != ESP_OK) {
        ESP_LOGE("app_main", "Error al inicializar MAX30102");
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    // Crear semáforos y colas
    sem_spo2_done = xSemaphoreCreateBinary();
    sem_temp_done = xSemaphoreCreateBinary();
    sensor_data_queue = xQueueCreate(130, sizeof(SensorData));
    oled_data_queue = xQueueCreate(130, sizeof(CharSensorData));

    
    /*rtc_time_t time;
    time.year = 2025;
    time.month = 10;
    time.day = 0;
    time.date = 8;
    time.hour = 13;
    time.min = 6;
    time.sec = 0;

    ds3231_set_time(&time);*/

    // Crear tareas
    xTaskCreate(sensor_temp, "sensor_temp", 4096, NULL, 2, &task[0].handle);
    xTaskCreate(sensor_SpO2, "sensor_SpO2", 8192, NULL, 2, &task[1].handle);
    xTaskCreate(cycle_controller_task, "cycle_controller_task", 4096, NULL, 3, NULL);
    xTaskCreate(procesar_datos, "procesar_datos", 4096, NULL, 1, NULL);
    xTaskCreate(desplegar_datos, "desplegar_datos", 4096, NULL, 1, NULL);
    ESP_LOGI("app_main", "System initialized, cycle controller task created");
}
