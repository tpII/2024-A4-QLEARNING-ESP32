#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

// Estructura para almacenar el estado del encoder
typedef struct {
    int pin_out;               // Pin de salida con interrupción
    volatile int32_t count;    // Contador de pulsos
} encoder_t;

// Prototipos de funciones
void encoder_init(encoder_t *encoder, int pin_out);
int32_t encoder_get_count(encoder_t *encoder);
void encoder_reset_count(encoder_t *encoder);
void encoder_isr_handler(void *arg);

#endif // ENCODER_H