#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2c.h"
#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "i2c_api.h"
#include "ssd1306.h"
#include "esp_log.h"
#include <esp_check.h>
#include "config_max.h"
#include "max30102_api.h"

#define I2C_MASTER_SCL_IO    SCL_PIN    // Pin de reloj I2C
#define I2C_MASTER_SDA_IO    SDA_PIN    // Pin de datos I2C
#define I2C_MASTER_NUM       I2C_NUM_0  // Número de bus I2C 

static const char *APP_TAG = "i2c-slave";

i2c_master_bus_handle_t bus_handle = NULL;
i2c_master_dev_handle_t max30102_handle = NULL;
i2c_master_dev_handle_t ssd1306_handle = NULL;
//i2c_master_dev_handle_t ds3231_handle = NULL;
ssd1306_config_t ssd1306_dim     = I2C_SSD1306_128x32_CONFIG_DEFAULT;

i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true, };


esp_err_t i2c_creation(void)
{
    ESP_LOGI(APP_TAG, "Inicializando bus I2C");

    esp_err_t err =ESP_OK;

    if(bus_handle == NULL){
        err = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
        if(err != ESP_OK){
            ESP_LOGE(APP_TAG, "Error creando bus I2C: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(APP_TAG, "Bus I2C creado");
        }
    }

    return err;
}


esp_err_t i2c_init(void){
    ESP_LOGI(APP_TAG, "Inicializando MAX30102");

    esp_err_t err;

    if(bus_handle == NULL){
        /*esp_err_t err = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
        if(err != ESP_OK){
            ESP_LOGE(APP_TAG, "Error creando bus I2C: %s", esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(APP_TAG, "Bus I2C creado");*/
        i2c_creation();
    }
    else{ESP_LOGI(APP_TAG, "Bus I2C ya creado.");}

        // Configurar dispositivo MAX30102
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = MAX30102_ADDR,
            .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        };
        
    err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &max30102_handle);
        if(err != ESP_OK){
            ESP_LOGE(APP_TAG, "Error agregando MAX30102: %s", esp_err_to_name(err));
            return err;
        }
        else{
            ESP_LOGI(APP_TAG, "MAX30102 agregado al bus I2C");
            max30102_init(&max30102_configuration);
            return err;
        }
}

// -----------------------------------
// Inicializa SSD1306 en el mismo bus
// -----------------------------------
esp_err_t i2c0_init(void){
    ESP_LOGI(APP_TAG, "Inicializando OLED SSD1306");

    esp_err_t err;
    if(bus_handle == NULL){
        /*esp_err_t err = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
        if(err != ESP_OK){
            ESP_LOGE(APP_TAG, "Error creando bus I2C: %s", esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(APP_TAG, "Bus I2C creado");*/
        i2c_creation();
    }
    else{ESP_LOGI(APP_TAG, "Bus I2C ya creado");}

    // Configurar dispositivo SSD1306
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SSD1306_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &ssd1306_handle);
    if(err != ESP_OK){
        ESP_LOGE(APP_TAG, "Error agregando SSD1306: %s", esp_err_to_name(err));
        return err;
    }
    else{
    // Inicializar pantalla
    err = ssd1306_init(bus_handle, &ssd1306_dim, &dev_hdl);
    if(err != ESP_OK || dev_hdl == NULL){
        ESP_LOGE(APP_TAG, "Error inicializando OLED SSD1306");
        return ESP_FAIL;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); // Delay antes de enviar datos
    ssd1306_clear_display(dev_hdl, false);
    ssd1306_display_text_x3(dev_hdl, 0, "OLED OK", false);

    ESP_LOGI(APP_TAG, "OLED SSD1306 inicializada correctamente");
    return ESP_OK;
    }
}

bool verificar(void){
    uint8_t part_id_reg = 0xFF;
    uint8_t respuesta = 0x00;
    esp_err_t err;

    // Enviar la dirección del registro PART_ID
    err = i2c_master_transmit_receive(max30102_handle, &part_id_reg, 1, &respuesta, 1, pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        ESP_LOGE("I2C", "Error al leer PART_ID: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI("I2C", "PART_ID leído: 0x%02X", respuesta);

    // Verificamos si coincide con el ID esperado
    if (respuesta == 0x15) {
        ESP_LOGI("I2C", "MAX30102 detectado correctamente.");
        return true;
    } else if (respuesta == 0x11) {
        ESP_LOGI("I2C", "Parece un MAX30100.");
        return true;
    } else {
        ESP_LOGW("I2C", "Dispositivo desconocido o respuesta inválida.");
        return false;
    }

}

esp_err_t i2c_sensor_read(uint8_t *data_rd, size_t size)
{
    return i2c_master_receive(max30102_handle, data_rd, size, pdMS_TO_TICKS(100));
}

esp_err_t i2c_sensor_write(uint8_t *data_wr, size_t size)
{
     return i2c_master_transmit(max30102_handle, data_wr, size, pdMS_TO_TICKS(100));
}


// -----------------------------------
// Inicializa DS3231 en el mismo bus
// -----------------------------------
esp_err_t i2c_init_ds3231(void){
    ESP_LOGI(APP_TAG, "Inicializando DS3231");

    esp_err_t err;

    if(bus_handle == NULL){
        /*esp_err_t err = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
        if(err != ESP_OK){
            ESP_LOGE(APP_TAG, "Error creando bus I2C: %s", esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(APP_TAG, "Bus I2C creado");*/
        i2c_creation();
    }
    else{ESP_LOGI(APP_TAG, "Bus I2C ya creado");}

    // Configurar dispositivo DS3231
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DS3231_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &ds3231_handle);
    if(err != ESP_OK){
        ESP_LOGE(APP_TAG, "Error agregando DS3231: %s", esp_err_to_name(err));
        return err;
    }
    else{
        ESP_LOGI(APP_TAG, "DS3232 inicializado correctamente");
        return ESP_OK;
    }
}