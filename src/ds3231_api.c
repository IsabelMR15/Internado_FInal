#include <stdio.h>
#include <esp_err.h>
#include "i2c_api.h"
#include "ds3231_api.h"

static uint8_t bcd2dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

static uint8_t dec2bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

esp_err_t ds3231_write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    return i2c_master_transmit(ds3231_handle, buf, sizeof(buf), -1);
}

esp_err_t ds3231_read_registers(uint8_t reg, uint8_t *data, size_t len) {
    esp_err_t err = i2c_master_transmit(ds3231_handle, &reg, 1, -1);
    if (err != ESP_OK) return err;
    return i2c_master_receive(ds3231_handle, data, len, -1);
}

esp_err_t ds3231_get_time(rtc_time_t *time) {
    uint8_t data[7];
    esp_err_t err = ds3231_read_registers(0x00, data, 7);
    if (err != ESP_OK) return err;

    time->sec   = bcd2dec(data[0] & 0x7F);
    time->min   = bcd2dec(data[1]);
    time->hour  = bcd2dec(data[2] & 0x3F);
    time->day   = bcd2dec(data[3]);
    time->date  = bcd2dec(data[4]);
    time->month = bcd2dec(data[5] & 0x1F);
    time->year  = 2000 + bcd2dec(data[6]);

    return ESP_OK;
}

esp_err_t ds3231_set_time(rtc_time_t *time) {
    uint8_t data[8];
    data[0] = 0x00; // registro de segundos
    data[1] = dec2bcd(time->sec);
    data[2] = dec2bcd(time->min);
    data[3] = dec2bcd(time->hour);
    data[4] = dec2bcd(time->day);
    data[5] = dec2bcd(time->date);
    data[6] = dec2bcd(time->month);
    data[7] = dec2bcd(time->year - 2000);

    return i2c_master_transmit(ds3231_handle, data, sizeof(data), -1);
}
