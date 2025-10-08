#include "Comunicaciones.h"

//static const char *TAG = "i2c-slave";

Comunicaciones::Comunicaciones () {
    
}

Comunicaciones::~Comunicaciones () {
    
}

esp_err_t Comunicaciones::i2c_slave_init(void){

    i2c_config_t conf_slave = {
        .mode = I2C_MODE_SLAVE,        
        .sda_io_num = I2C_SLAVE_SDA_IO, // select GPIO sda
        .scl_io_num = I2C_SLAVE_SCL_IO, // select GPIO scl        
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave={.addr_10bit_en = 0, .slave_addr = ESP_SLAVE_ADDR},
        .clk_flags = 0,
    };
    esp_err_t err = i2c_param_config(i2c_slave_port, &conf_slave);
    if (err != ESP_OK){
        return err;
    }
    return i2c_driver_install(i2c_slave_port, conf_slave.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}