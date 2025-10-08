
#ifndef ALGORITHM_H
#define ALGORITHM_H

//#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void remove_dc_part(int32_t *ir_buffer, int32_t *red_buffer, uint64_t *ir_mean, uint64_t *red_mean);

int calculate_heart_rate(int32_t *red_data);
double spo2_measurement(int32_t *ir_data, int32_t *red_data);
void moving_average_filter(int32_t *ir_data, int32_t *red_data);
int count_peaks(int32_t *data,int threshold, uint16_t *peak_indices);
double calculate_average(int32_t *data, int tam) ;
double rms_value(int32_t *data, int tam);
void filtroPasaBajasPrimerOrden(int32_t *ir_buffer, int32_t *red_buffer);

int calcular_heart_rate(float *data, int tam);
float normalizar(int32_t *data, int a, int32_t min, int32_t max);
int32_t calcular_min(int32_t *data, int tam);
int32_t calcular_max(int32_t *data, int tam);
int count_peaksf(float *data,float threshold, uint16_t *peak_indices, int tam);
int32_t crearArreglo(int cont, int32_t *data);
void moving_average_muestreo(int32_t *nuevo, int tam);
float rms_valuef(float *data, int tam);
float calculate_averagef(float *data, int tam );
void filtroPasaBajasPrimerOrdenmuestreo(int32_t *nuevo);

#define BUFFER_SIZE 64 //128
#define DELAY_AMOSTRAGEM 40

#ifdef __cplusplus
}
#endif

#endif
