#ifndef COMUNICACIONES_H_INCLUDED
#define COMUNICACIONES_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_SLAVE_SCL_IO GPIO_NUM_33 //33      /*!< gpio number for I2C master clock */
#define I2C_SLAVE_SDA_IO GPIO_NUM_32  /*!< gpio number for I2C master data  */
#define I2C_SLAVE_FREQ_HZ 400000 /*!< I2C master clock frequency */
#define I2C_SLAVE_TX_BUF_LEN 255 /*!< I2C master doesn't need buffer */
#define I2C_SLAVE_RX_BUF_LEN 255 /*!< I2C master doesn't need buffer */
#define ESP_SLAVE_ADDR 0x0A

class Comunicaciones {
    i2c_port_t i2c_master_port =I2C_NUM_0;
    i2c_port_t i2c_slave_port = I2C_NUM_0;
    const char *TAG = "i2c-slave";

    public:
        Comunicaciones();
        virtual ~Comunicaciones();
        esp_err_t i2c_slave_init(void);
        esp_err_t i2c_master_send(uint8_t message[], int len);
        esp_err_t i2c_master_init(void);
    protected:

    private:

};

#endif