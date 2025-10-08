#ifndef SSD_OLED_H
#define SS_OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306.h"
#include "string.h"
#include "../lib/libsd/SDManager.h"

void display_max30102(ssd1306_handle_t dev_hdl, CharSensorData sent);
void display_DS18B20(ssd1306_handle_t dev_hdl, CharSensorData sent);
void errorMessage(ssd1306_handle_t dev_hdl);
void Init_Message(ssd1306_handle_t dev_hdl);
void message_HR(ssd1306_handle_t dev_hdl);
void message_temp(ssd1306_handle_t dev_hdl);

#ifdef __cplusplus
}
#endif

#endif 