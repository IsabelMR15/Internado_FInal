#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h" 
#include "esp_rom_sys.h"

#define ONE_WIRE_PIN GPIO_NUM_14

void one_wire_write_bit(bool bit) {
    gpio_set_direction(ONE_WIRE_PIN, GPIO_MODE_OUTPUT); // Configurar el bit como de salida
    
    if (bit) {
        gpio_set_level(ONE_WIRE_PIN, 0);
        esp_rom_delay_us(10);                   // Mantiene bajo por 10 us
        gpio_set_level(ONE_WIRE_PIN, 1);
        esp_rom_delay_us(55);                   // Libera la línea por el resto del tiempo
    } else {
        gpio_set_level(ONE_WIRE_PIN, 0);
        esp_rom_delay_us(65);               
        gpio_set_level(ONE_WIRE_PIN, 1);
    }

    esp_rom_delay_us(5);  // Espera en caso de escribir otro bit
}

extern void one_wire_write_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) { //Recorre los 8 bits del byte
        one_wire_write_bit(data & 0x01);  // Envía el bit menos significativo
        data >>= 1;                       // Desplaza los bits a la derecha
    }
}
