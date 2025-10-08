#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h" 
#include "esp_rom_sys.h"

#define ONE_WIRE_PIN GPIO_NUM_14

bool detect_sensor() {
    bool isPresent = false;

    // Configurar el pin como salida y enviar el pulso de reinicio
    gpio_set_direction(ONE_WIRE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(ONE_WIRE_PIN, 0);
    esp_rom_delay_us(480);  // Mantener la línea baja por 480 us

    // Cambiar el pin a modo entrada con pull-up y esperar la respuesta del sensor
    gpio_set_direction(ONE_WIRE_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ONE_WIRE_PIN, GPIO_PULLUP_ONLY);
    esp_rom_delay_us(90);  // Esperar tiempo suficiente para detectar el pulso de presencia

    // Leer el estado del pin para determinar si hay un sensor presente
    isPresent = (gpio_get_level(ONE_WIRE_PIN) == 0);
    esp_rom_delay_us(410);  // Completar el tiempo total del reset pulse (960 us)

    return isPresent;  // Devolver true si hay sensor, false si no
}