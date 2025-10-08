#include "main.h"
#include "ssd1306.h"
#include "string.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <esp_check.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *APP_TAG2 = "i2c-slave-OLED";

void display_max30102(ssd1306_handle_t dev_hdl, CharSensorData sent)
{
    char chain_hr[40] ="HR:";
    char chain_spo2[40] = "SpO2:";

    strcat(chain_hr, sent.c_heart_rate);
    strcat(chain_spo2, sent.c_spo2);
    ESP_LOGI(APP_TAG2, "Display x3 Text");
    ssd1306_clear_display(dev_hdl, false);
    ssd1306_set_contrast(dev_hdl, 0xFF);
    ssd1306_display_text_x2(dev_hdl, 0, chain_hr, false);
    ssd1306_display_text_x2(dev_hdl, 2, chain_spo2, false);

    vTaskDelay(pdMS_TO_TICKS(500)); // medio segundo
}

void display_DS18B20(ssd1306_handle_t dev_hdl, CharSensorData sent)
{
    char chain_temp[40] ="Temp:";
    char chain_space[40] = "   ";

    strcat(chain_space, sent.c_temp_float);
    ESP_LOGI(APP_TAG2, "Display x3 Text");
    ssd1306_clear_display(dev_hdl, false);
    ssd1306_set_contrast(dev_hdl, 0xFF);
    ssd1306_display_text_x2(dev_hdl, 0, chain_temp, false);
    ssd1306_display_text_x2(dev_hdl, 2, chain_space, false);

    vTaskDelay(pdMS_TO_TICKS(500)); // medio segundo
}

void errorMessage(ssd1306_handle_t dev_hdl){
    
    ssd1306_clear_display(dev_hdl, false);
    ssd1306_set_contrast(dev_hdl, 0xff);
    ssd1306_display_text_x2(dev_hdl, 0, "Coloque", false);
    ssd1306_display_text_x2(dev_hdl, 2, "  dedo", false);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
}

void Init_Message(ssd1306_handle_t dev_hdl){
    
    ssd1306_clear_display(dev_hdl, false);
    ssd1306_set_contrast(dev_hdl, 0xff);
    ssd1306_display_text(dev_hdl, 0, "   Bienvenido", false);
    ssd1306_display_text(dev_hdl, 1, "   Mediciones", false);
    ssd1306_display_text(dev_hdl, 2, "   de Spo2,HR", false);
    ssd1306_display_text(dev_hdl, 3, " y Temperatura", false);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
}


void message_HR(ssd1306_handle_t dev_hdl){
    
    ssd1306_clear_display(dev_hdl, false);
    ssd1306_set_contrast(dev_hdl, 0xff);
    ssd1306_display_text(dev_hdl, 1, "Midiendo", false);
    ssd1306_display_text(dev_hdl, 2, "SpO2 y FC", false);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
}

void message_temp(ssd1306_handle_t dev_hdl){
    
    ssd1306_clear_display(dev_hdl, false);
    ssd1306_set_contrast(dev_hdl, 0xff);
    ssd1306_display_text(dev_hdl, 1, "Midiendo", false);
    ssd1306_display_text(dev_hdl, 2, "Temperatura", false);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
}
