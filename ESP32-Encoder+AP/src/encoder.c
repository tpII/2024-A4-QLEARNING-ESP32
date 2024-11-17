#include "encoder.h"
#include "driver/gpio.h"

#include "esp_timer.h"  // Para obtener marca de tiempo en microsegundos

// Variables globales para comparar pulsos
static volatile int64_t encoder1_last_time = 0;
static volatile int64_t encoder2_last_time = 0;
static volatile direction_t movement_direction = DIRECTION_STOPPED;



// ISR: Incrementa el contador cada vez que se genera un pulso
void encoder_isr_handler(void *arg) {
    encoder_t *encoder = (encoder_t *)arg;
    int64_t now = esp_timer_get_time();  // Marca de tiempo en microsegundos
    encoder->count++;  // Incrementar el contador de pulsos
    encoder->last_pulse_time = now;

    // Comparar tiempos para determinar la dirección
    if (encoder->pin_out == ENCODER1_OUT) {  // Encoder 1
        encoder1_last_time = now;
        if (encoder1_last_time > encoder2_last_time) {
            movement_direction = DIRECTION_FORWARD;
        } else if (encoder1_last_time < encoder2_last_time) {
            movement_direction = DIRECTION_BACKWARD;
        }
    } else if (encoder->pin_out == ENCODER2_OUT) {  // Encoder 2
        encoder2_last_time = now;
        if (encoder2_last_time > encoder1_last_time) {
            movement_direction = DIRECTION_BACKWARD;
        } else if (encoder2_last_time < encoder1_last_time) {
            movement_direction = DIRECTION_FORWARD;
        }
    }
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

direction_t get_movement_direction() {
    return movement_direction;
}

