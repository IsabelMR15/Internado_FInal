#ifndef I2C_API_H
#define I2C_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "ssd1306.h"

#define MAX30102_ADDR 0x57 /// Direccion del sensor MAX30102
#define SSD1306_ADDR  0x3C /// Direccion del display SSD1306
#define DS3231_ADDR 0x68 /// Dirección de RTC DS3231
#define I2C_MASTER_FREQ_HZ  400000 

#define ACK_CHECK_EN 0x1
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define ACK_VAL 0x0
#define NACK_VAL 0x1

#define SDA_PIN 9
#define SCL_PIN 8

extern ssd1306_handle_t dev_hdl; // handle global de la OLED
extern i2c_master_dev_handle_t ds3231_handle;

esp_err_t i2c_creation(void);
esp_err_t i2c_init(void);
esp_err_t i2c0_init(void);
esp_err_t i2c_sensor_read(uint8_t *data_rd, size_t size);
esp_err_t i2c_sensor_write(uint8_t *data_wr, size_t size);
bool verificar(void);
esp_err_t i2c_init_ds3231(void);


#ifdef __cplusplus
}
#endif

#endif 