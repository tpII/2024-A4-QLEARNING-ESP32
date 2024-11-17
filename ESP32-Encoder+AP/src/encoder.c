#include "encoder.h"
#include "driver/gpio.h"

// ISR: Incrementa el contador cada vez que se genera un pulso
void encoder_isr_handler(void *arg) {
    encoder_t *encoder = (encoder_t *)arg;  // Obtener la estructura del encoder
    encoder->count++;  // Incrementar el contador por cada pulso detectado
}

// Inicializa un encoder con su pin de salida (OUT)
void encoder_init(encoder_t *encoder, int pin_out) {
    encoder->pin_out = pin_out;
    encoder->count = 0;

    // Configurar el pin como entrada con interrupción en cualquier cambio de estado
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,  // Interrupción en flanco de subida
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << pin_out),
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    // Instalar el servicio de interrupciones (una sola vez)
    gpio_install_isr_service(0);

    // Añadir la interrupción para el pin de salida del encoder
    gpio_isr_handler_add(pin_out, encoder_isr_handler, (void *)encoder);
}

// Devuelve el valor del contador del encoder
int32_t encoder_get_count(encoder_t *encoder) {
    return encoder->count;
}

// Resetea el contador del encoder a cero
void encoder_reset_count(encoder_t *encoder) {
    encoder->count = 0;
}
