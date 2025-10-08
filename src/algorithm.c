#include "algorithm.h"
#include <math.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"


#define DEBUG true // Constante que indica si se activa el modo de depuración 

void filtroPasaBajasPrimerOrden(int32_t *ir_buffer, int32_t *red_buffer){
	float alpha = 0.5;

	for(int i=0; i<BUFFER_SIZE; i++)
	{
		// Ecuación diferencial: y(n) = alpha * x(n) + (1 - alpha) * y(n-1)
		if(i == 0){
			ir_buffer[i] = ir_buffer[i];
			red_buffer[i] = red_buffer[i];
		}
		else{
			ir_buffer[i] = alpha * ir_buffer[i] + (1 - alpha) * ir_buffer[i-1];
			red_buffer[i] = alpha * red_buffer[i] + (1 - alpha) * red_buffer[i-1];
		}
	}
}

void remove_dc_part(int32_t *ir_buffer, int32_t *red_buffer, uint64_t *ir_mean, uint64_t *red_mean){
	*ir_mean = 0;
	*red_mean = 0;
	for(int i = 0; i < BUFFER_SIZE; i++){
		*ir_mean += ir_buffer[i];
	    *red_mean += red_buffer[i];
	}

	*ir_mean = *ir_mean / (BUFFER_SIZE);
	*red_mean = *red_mean / (BUFFER_SIZE);
}

void moving_average_filter(int32_t *ir_data, int32_t *red_data){
 	int window_size = 3; //3
	int32_t sum_ir=0;
	int count_ir = 0;
	int32_t sum_red=0;
	int count_red = 0;

    for (int i = 0; i < BUFFER_SIZE ; i++) {
		sum_ir = 0;
		count_ir = 0;
        for (int j = i; j >= 0 && j > i - window_size; j--) {
            sum_ir += ir_data[j];
            count_ir++;
        }
		if(i==2){
			ir_data[i]= ir_data[i-1];
		}
		else{ir_data[i] = sum_ir / count_ir;} // Promedio
    }

	for (int i = 0; i < BUFFER_SIZE ; i++) {
		sum_red = 0;
		count_red = 0;
        for (int j = i; j >= 0 && j > i - window_size; j--) {
            sum_red += red_data[j];
            count_red++;
        }
		if(i==2){
			red_data[i]= red_data[i-1];
		}
		else{
			red_data[i] = sum_red / count_red;
		}
    }
	/*for (int i = 0; i < BUFFER_SIZE; i++) 
	{printf("IR fil: %ld \t RED fil: %ld\n", ir_data[i], red_data[i]);}*/
	vTaskDelay(1000); //1 segundos
}


double spo2_measurement(int32_t *ir_data, int32_t *red_data){
	double Z = 0;
	double z2 =0;
	double SpO2;
	double sat;
	double ir_rms; //rms_value(ir_data);
	double red_rms; // rms_value(red_data);
	double ir_mean1 = 0;
	double red_mean1 = 0;
	int32_t ir_buff[BUFFER_SIZE];
	int32_t red_buff[BUFFER_SIZE];

	for(int i = 0; i < BUFFER_SIZE; i++){
		ir_mean1 += ir_data[i];
	    red_mean1 += red_data[i];
		//printf("IR: %ld \t RED: %ld\n", ir_data[i], red_data[i]);
	}

	ir_mean1 = ir_mean1 / (BUFFER_SIZE);
	red_mean1 = red_mean1 / (BUFFER_SIZE); // Calcula la media

	ir_rms = rms_value(ir_buff, BUFFER_SIZE);
	red_rms = rms_value(red_buff, BUFFER_SIZE);

	Z = (red_rms/red_mean1)/ (ir_rms/ir_mean1);
	z2 = (ir_rms/ir_mean1)/(red_rms/red_mean1);
	SpO2=117-(25*Z); 
	sat = (Z-10.006)/(-0.0965);
	 	

	/*printf("red_rms %f\n", red_rms);
	printf("ir_rms %f\n", ir_rms);
	printf("ir_mean %f\n", ir_mean1);
	printf("red_mean %f\n", red_mean1);*/
	printf("Z %f\n", Z);
	printf("z2 %f\n", z2);
	printf("sat %f\n", sat);

	return round(SpO2);
}


int calculate_heart_rate(int32_t *red_data){
	int resultado= 0;
	int umbral = 0;
	int peaks = 0;
	uint16_t peak_indices[BUFFER_SIZE] = {0};

	umbral = (int)(calculate_average(red_data, BUFFER_SIZE) + 0.51 * rms_value(red_data, BUFFER_SIZE)); //.51
	//printf("media %f\n", calculate_average(red_buff));
	//printf("rms %f\n", rms_value(red_buff));
	//printf("umbral %d\n", umbral);
	peaks = count_peaks(red_data, umbral, peak_indices);

	//printf("peaks %d\n", peaks);
	/*for(int j=0; j< peaks; j++)
	{printf("posición peak %d: %d\n", j, peak_indices[j]);}*/

	float sum_diff = 0;
	float total_interval_ms = 0;

	for(int i= 1; i< peaks; i++)
	{
		sum_diff =(peak_indices[i] - peak_indices[i-1]);
		total_interval_ms += sum_diff * DELAY_AMOSTRAGEM;
	}

	float avg_interval_sec = (total_interval_ms / (peaks - 1)) / 1000.0;
	//printf("avg_interval_sec: %f\n", avg_interval_sec);
    float bpm = 60.0 / avg_interval_sec;

	resultado = round(bpm);

	return resultado;
}


double rms_value(int32_t *data, int tam){
	int32_t somatoria = 0;
	double data_mean=0;

	for(int i = 0; i < tam; i++){
		data_mean+= data[i];
	}
	data_mean = data_mean / (tam);
    for (int i = 0; i < tam; i++) {
        somatoria += pow(data[i] - data_mean, 2);
    }
    return sqrt(somatoria / tam);
}

double calculate_average(int32_t *data, int tam ){
    double somatoria = 0;

    for (int i = 0; i < tam; i++) {
        somatoria += data[i];
    }
    return somatoria / tam;
}


int count_peaks(int32_t *data,int threshold, uint16_t *peak_indices) {
    int peak_count = 0;
	int j=0;
    for (int i = 1; i < BUFFER_SIZE -1; i++) {
        if (data[i] > threshold && data[i] > data[i-1] && data[i] > data[i+1]) 
		{
            peak_count++;
			peak_indices[j] = i;
            i += 5; // Evita detecciones muy cercanas entre sí
			j++;
        }
    }
    return peak_count;
}


float normalizar(int32_t *data, int a, int32_t min, int32_t max){
	float norm=0;
	float minf, maxf,dato;

	minf = (float)min;
	maxf = (float)max;
	dato = (float)data[a];

	if(maxf != minf)
	{norm = (dato - minf) / (maxf - minf);}
	
	else
	{norm = 0;}
	
	return norm;
}

int calcular_heart_rate(float *data, int tam){
	int resultado= 0;
	float umbral;;
	int peaks = 0;
	uint16_t peak_indices[BUFFER_SIZE] = {0};

	if(tam == BUFFER_SIZE){
		//umbral=0.9750;
		umbral = calculate_averagef(data, BUFFER_SIZE) + 0.2 * rms_valuef(data, BUFFER_SIZE);}
	else{
		//umbral = 0.9950;
		umbral = calculate_averagef(data, 16) + 0.19* rms_valuef(data, 16);
		}

	printf("umbral %f\n", umbral);
	peaks = count_peaksf(data, umbral, peak_indices,tam);

	//printf("peaks %d\n", peaks);
	/*for(int j=0; j< peaks; j++)
	{printf("posición peak %d: %d\n", j, peak_indices[j]);}*/

	float sum_diff = 0;
	float total_interval_ms = 0;

	if(peaks > 1){
		for(int i= 1; i< peaks; i++)
		{
			sum_diff =(peak_indices[i] - peak_indices[i-1]);
			if(tam==BUFFER_SIZE){total_interval_ms += sum_diff * DELAY_AMOSTRAGEM;}
			else{total_interval_ms += sum_diff * (4*DELAY_AMOSTRAGEM);}
		}

		float avg_interval_sec = (total_interval_ms / (peaks - 1)) / 1000.0;
		//printf("avg_interval_sec: %f\n", avg_interval_sec);
    	float bpm = 60.0 / avg_interval_sec;

		resultado = round(bpm);
	}
	else{resultado = 0;}

	return resultado;
}

int count_peaksf(float *data,float threshold, uint16_t *peak_indices, int tam) {
	
	int peak_count = 0;
	int j=0;
    for (int i = 2; i < tam -1; i++) {
        if (data[i] > threshold && data[i] > data[i-1] && data[i] > data[i+1]) 
		{
			//printf("data[i]: %f\n", data[i]);
            peak_count++;
			peak_indices[j] = i;
			if(tam == BUFFER_SIZE){i += 8;}
			else{i=i;} // Evita detecciones muy cercanas entre sí
			j++;
        }
    }
    return peak_count;
}

int32_t calcular_min(int32_t *data, int tam){
	int rep =tam;
	int32_t min = data[0];
	for(int i=1; i<rep; i++)
	{
		if(data[i] < min)
		{
			min = data[i];
		}
	}
	return min;
}

int32_t calcular_max(int32_t *data, int tam){
	int rep =tam;
	int32_t max = data[0];
	for(int i=1; i<rep; i++)
	{
		if(data[i] > max)
		{
			max = data[i];
		}
	}
	return max;
}

int32_t crearArreglo(int cont, int32_t *data) {
	int h;
	int32_t sum=0;
	int a=0;
	int32_t prom;

	a=cont*4;
	h=0;
	  while(h<4)
	  {
		  sum += data[a];
		  h++;
		  a++;
	  }

	  prom= sum/4;
	  sum=0;
	return prom;
   }


void moving_average_muestreo(int32_t *nuevo, int tam){
 	int window_size = 2; //3
	int32_t sum_n=0;
	int count_n = 0;

    for (int i = 0; i < tam ; i++) {
		sum_n = 0;
		count_n = 0;
        for (int j = i; j >= 0 && j > i - window_size; j--) {
            sum_n += nuevo[j];
            count_n++;
        }
		if(i==2){
			nuevo[i]= nuevo[i-1];
		}
		else{nuevo[i] = sum_n / count_n;} // Promedio
    }
	/*for (int i = 0; i < BUFFER_SIZE; i++) 
	{//printf("IR fil: %ld \t RED fil: %ld\n", ir_data[i], red_data[i]);}*/
	//printf("\n");
	vTaskDelay(1000); //1 segundos
}


float rms_valuef(float *data, int tam){
	float somatoria = 0;
	float data_mean=0;

	for(int i = 0; i < tam; i++){
		data_mean+= data[i];
	}
	data_mean = data_mean / (tam);
    for (int i = 0; i < tam; i++) {
        somatoria += pow(data[i] - data_mean, 2);
    }
    return sqrt(somatoria / tam);
}

float calculate_averagef(float *data, int tam ){
    float somatoria = 0;

    for (int i = 0; i < tam; i++) {
        somatoria += data[i];
    }
    return somatoria / tam;
}  


void filtroPasaBajasPrimerOrdenmuestreo(int32_t *nuevo){
	float alpha = 0.5;

	for(int i=0; i<16; i++)
	{
		// Ecuación diferencial: y(n) = alpha * x(n) + (1 - alpha) * y(n-1)
		if(i == 0){
			nuevo[i] = nuevo[i];
		}
		else{
			nuevo[i] = alpha * nuevo[i] + (1 - alpha) * nuevo[i-1];}
	}
}