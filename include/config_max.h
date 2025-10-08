
#ifndef CONFIG_MAX_H
#define CONFIG_MAX_H

#include "max30102_api.h"

#ifdef __cplusplus
extern "C" {
#endif

//Configuración del sensor MAX30102 en modo SPO2
max_config max30102_configuration = {
    .INT_EN_1 = {
        .A_FULL_EN = 1,
        .PPG_RDY_EN = 1,
        .ALC_OVF_EN = 0,
        .PROX_INT_EN = 0,
		.RESERVED = 0,
    },
    .INT_EN_2 = {
        .DIE_TEMP_RDY_EN = 0,
		.RESERVED1 = 0,
    },
    .FIFO_WRITE_PTR = {
        .FIFO_WR_PTR = 0,
		.RESERVED = 0,
    },
    .OVEF_COUNTER = {
        .OVF_COUNTER = 0,
		.RESERVED = 0,
    },
    .FIFO_READ_PTR = {
        .FIFO_RD_PTR = 0,
		.RESERVED = 0,
    },
    .FIFO_CONF = {
        .FIFO_A_FULL = 0,          // Inicializa FIFO_A_FULL
        .FIFO_ROLLOVER_EN = 1,     // Habilitar FIFO rollover
        .SMP_AVE = 0b010,          // Promedio de 4 valores
    },
    .MODE_CONF = {
		.MODE = 0b011,              // Modo SPO2 0b011
		.RESERVED = 0,
        .RESET = 0,
		.SHDN = 0,
    },
    .SPO2_CONF = {
		.LED_PW = 0b10,      // Pulso de 215 µs del LED 0b10
		.SPO2_SR = 0b001    ,           // 200 muestras por segundo 0b001
        .SPO2_ADC_RGE = 0b01,   // 16384 nA (Escala del DAC) 0b01
		.RESERVED = 0,
    },
    .LED1_PULSE_AMP = {
        .LED1_PA = 0x24,          // Corriente del LED1 25.4 mA
    },
    .LED2_PULSE_AMP = {
        .LED2_PA = 0x24,      // Corriente del LED2 25.4 mA
    },
    .PROX_LED_PULS_AMP = {
        .PILOT_PA = 0x7F,           // Amplitud del LED de proximidad
    },
    .MULTI_LED_CONTROL1 = {
        .SLOT1 = 0,                 // Deshabilitado
		.RESERVED2 = 0,
        .SLOT2 = 0,                 // Deshabilitado
		.RESERVED1 = 0,
    },
    .MULTI_LED_CONTROL2 = {
        .SLOT3 = 0,                 // Deshabilitado
		.RESERVED2 = 0,
        .SLOT4 = 0,                 // Deshabilitado
		.RESERVED1 = 0,
    },
};

//Configuración del sensor MAX30102 en modo HR
/*max_config max30102_configuration_HR ={
    .INT_EN_1 = {
        .A_FULL_EN = 1,
        .PPG_RDY_EN = 1,
        .ALC_OVF_EN = 0,
        .PROX_INT_EN = 0,
		.RESERVED = 0,
    },
    .INT_EN_2 = {
        .DIE_TEMP_RDY_EN = 0,
		.RESERVED1 = 0,
    },
    .FIFO_WRITE_PTR = {
        .FIFO_WR_PTR = 0,
		.RESERVED = 0,
    },
    .OVEF_COUNTER = {
        .OVF_COUNTER = 0,
		.RESERVED = 0,
    },
    .FIFO_READ_PTR = {
        .FIFO_RD_PTR = 0,
		.RESERVED = 0,
    },
    .FIFO_CONF = {
        .FIFO_A_FULL = 0,          // Inicializa FIFO_A_FULL
        .FIFO_ROLLOVER_EN = 1,     // Habilitar FIFO rollover
        .SMP_AVE = 0b010,          // Promedio de 4 valores
    },
    .MODE_CONF = {
		.MODE = 0b011,              // Modo HR 0b011
		.RESERVED = 0,
        .RESET = 0,
		.SHDN = 0,
    },
    .SPO2_CONF = {
		.LED_PW = 0b10,      // Pulso de 215 µs del LED 0b10
		.SPO2_SR = 0b001    ,           // 200 muestras por segundo 0b001
        .SPO2_ADC_RGE = 0b01,   // 16384 nA (Escala del DAC) 0b01
		.RESERVED = 0,
    },
    .LED1_PULSE_AMP = {
        .LED1_PA = 0x24,          // Corriente del LED1 25.4 mA, 0x24
    },
    .LED2_PULSE_AMP = {
        .LED2_PA = 0x24,            // Corriente del LED2 25.4 mA0x24
    },
    .PROX_LED_PULS_AMP = {
        .PILOT_PA = 0x7F,           // Amplitud del LED de proximidad
    },
    .MULTI_LED_CONTROL1 = {
        .SLOT1 = 0,                 // Deshabilitado
		.RESERVED2 = 0,
        .SLOT2 = 0,                 // Deshabilitado
		.RESERVED1 = 0,
    },
    .MULTI_LED_CONTROL2 = {
        .SLOT3 = 0,                 // Deshabilitado
		.RESERVED2 = 0,
        .SLOT4 = 0,                 // Deshabilitado
		.RESERVED1 = 0,
    },
};*/

#ifdef __cplusplus
}
#endif

#endif
