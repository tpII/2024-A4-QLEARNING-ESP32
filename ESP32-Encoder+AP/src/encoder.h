#ifndef ENCODER_H
#define ENCODER_H

// Definir los pines de salida de los encoders
#define ENCODER1_OUT 16
#define ENCODER2_OUT 17

#include <stdint.h>

// Estructura para almacenar el estado del encoder
typedef struct {
    int pin_out;               // Pin de salida con interrupción
    volatile int32_t count;    // Contador de pulsos
    volatile int64_t last_pulse_time;  // Marca de tiempo del último pulso
} encoder_t;

// Nueva enumeración para la dirección
typedef enum {
    DIRECTION_STOPPED = 0,     // Sin movimiento
    DIRECTION_FORWARD = 1,     // Movimiento hacia adelante
    DIRECTION_BACKWARD = -1    // Movimiento hacia atrás
} direction_t;

// Prototipo adicional
direction_t get_movement_direction();

// Prototipos de funciones
void encoder_init(encoder_t *encoder, int pin_out);
int32_t encoder_get_count(encoder_t *encoder);
void encoder_reset_count(encoder_t *encoder);
void encoder_isr_handler(void *arg);


#endif // ENCODER_H