#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h" 
#include "esp_rom_sys.h"

#define ONE_WIRE_PIN GPIO_NUM_14 

bool one_wire_read_bit(void) {
    bool bit;

    gpio_set_direction(ONE_WIRE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(ONE_WIRE_PIN, 0);    //  Lo determina a bajo
    esp_rom_delay_us(3);

    gpio_set_direction(ONE_WIRE_PIN, GPIO_MODE_INPUT);  // Libera la línea para leer el bit
    esp_rom_delay_us(10);
    
    bit = gpio_get_level(ONE_WIRE_PIN); // Lee el valor del bit

    esp_rom_delay_us(50); 
    return bit;
}

extern uint8_t one_wire_read_byte(void) {
    uint8_t data = 0;

    for (int i = 0; i < 8; i++) {
        data >>= 1;  // Recorre los bits a la derecha
        if (one_wire_read_bit()) {
            data |= 0x80;  //Si el bit es leído, coloca 1 en el bit más significativo
        }
    }

    return data;
}