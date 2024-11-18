#ifndef SERVO_H
#define SERVO_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

// Definir pines para los servos
#define SHOULDER_PIN    GPIO_NUM_13
#define ELBOW_PIN       GPIO_NUM_12

// Configuración del servo
#define SERVO_MIN_PULSEWIDTH             400     // Ancho de pulso para 0 grados en microsegundos
#define SERVO_MID_PULSEWIDTH             950     // Ancho de pulso para 45 grados en microsegundos
#define SERVO_MAX_PULSEWIDTH             1400    // Ancho de pulso para 90 grados en microsegundos
#define SERVO_FREQUENCY                  50      // 50Hz para servos SG90

// Configuración de LEDC para PWM
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Resolución de ciclo de trabajo de 13 bits
#define LEDC_SHOULDER_CHANNEL   LEDC_CHANNEL_0
#define LEDC_ELBOW_CHANNEL      LEDC_CHANNEL_1

// Configuración de ángulos
#define SERVO_MIN_ANGLE  0
#define SERVO_MID_ANGLE  45
#define SERVO_MAX_ANGLE  90
#define SERVO_STEP_ANGLE 45

// Declaraciones de funciones (prototipos)
void servo_init(void);
void set_servo_angle(ledc_channel_t channel, int angle);
uint32_t degree_to_duty(int angle);
void process_keypad(char key);
char get_keypad_input(void);

#endif // SERVO_H