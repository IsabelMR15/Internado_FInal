#ifndef DS3231_API_H
#define DS3231_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <esp_err.h>
#include "i2c_api.h"

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

static uint8_t bcd2dec(uint8_t val);
static uint8_t dec2bcd(uint8_t val);
esp_err_t ds3231_write_register(uint8_t reg, uint8_t data);
esp_err_t ds3231_read_registers(uint8_t reg, uint8_t *data, size_t len);
esp_err_t ds3231_get_time(rtc_time_t *time);
esp_err_t ds3231_set_time(rtc_time_t *time);


#ifdef __cplusplus
}
#endif

#endif 