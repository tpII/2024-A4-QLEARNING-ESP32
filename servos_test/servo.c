#include "servo.h"

// Variables globales
static int current_pos[2] = {0, 0}; // [hombro, codo]


// Función para convertir grados a ciclo de trabajo
uint32_t degree_to_duty(int angle) {
    int pulse_width;
    switch (angle) {
        case 0:
            pulse_width = SERVO_MIN_PULSEWIDTH;
            break;
        case 45:
            pulse_width = SERVO_MID_PULSEWIDTH;
            break;
        case 90:
            pulse_width = SERVO_MAX_PULSEWIDTH;
            break;
        default:
            pulse_width = SERVO_MID_PULSEWIDTH; // Valor por defecto
            break;
    }
    return (pulse_width * ((1 << LEDC_DUTY_RES) - 1)) / 20000;
}

// Función para establecer el ángulo del servo
void set_servo_angle(ledc_channel_t channel, int angle) {
    int duty = degree_to_duty(angle);
    ledc_set_duty(LEDC_MODE, channel, duty);
    ledc_update_duty(LEDC_MODE, channel);
}

// Función para inicializar los servos
void init_servo() {
    // Configurar temporizador para LEDC
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = SERVO_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configurar canal para el servo del hombro
    ledc_channel_config_t ledc_channel_shoulder = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_SHOULDER_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SHOULDER_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_shoulder);

    // Configurar canal para el servo del codo
    ledc_channel_config_t ledc_channel_elbow = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_ELBOW_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = ELBOW_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_elbow);
}

// Función para procesar la entrada del teclado
void process_keypad(char key) {
    switch (key) {
        case '8': // Arriba
            current_pos[0] += SERVO_STEP_ANGLE;
            if (current_pos[0] > SERVO_MAX_ANGLE) current_pos[0] = SERVO_MAX_ANGLE;
            set_servo_angle(LEDC_SHOULDER_CHANNEL, current_pos[0]);
            break;
        case '2': // Abajo
            current_pos[0] -= SERVO_STEP_ANGLE;
            if (current_pos[0] < SERVO_MIN_ANGLE) current_pos[0] = SERVO_MIN_ANGLE;
            set_servo_angle(LEDC_SHOULDER_CHANNEL, current_pos[0]);
            break;
        case '4': // Izquierda
            current_pos[1] += SERVO_STEP_ANGLE;
            if (current_pos[1] > SERVO_MAX_ANGLE) current_pos[1] = SERVO_MAX_ANGLE;
            set_servo_angle(LEDC_ELBOW_CHANNEL, current_pos[1]);
            break;
        case '6': // Derecha
            current_pos[1] -= SERVO_STEP_ANGLE;
            if (current_pos[1] < SERVO_MIN_ANGLE) current_pos[1] = SERVO_MIN_ANGLE;
            set_servo_angle(LEDC_ELBOW_CHANNEL, current_pos[1]);
            break;
        case '5': // Centro
            current_pos[0] = SERVO_MID_ANGLE;
            current_pos[1] = SERVO_MID_ANGLE;
            set_servo_angle(LEDC_SHOULDER_CHANNEL, current_pos[0]);
            set_servo_angle(LEDC_ELBOW_CHANNEL, current_pos[1]);
            break;
        default:
            break;
    }
}

// Función para obtener la entrada del teclado
char get_keypad_input() {
    char key;
    while (1) {
        scanf(" %c", &key);
        if (key == '8' || key == '2' || key == '4' || key == '6' || key == '5'){
            return key;
        } else {
            printf("Tecla inválida presionada\n");
        }
    }
}

// Función principal
void app_main() {
    // Inicializar servos
    init_servo();
    // Establecer ángulos iniciales
    set_servo_angle(LEDC_SHOULDER_CHANNEL, current_pos[0]);
    set_servo_angle(LEDC_ELBOW_CHANNEL, current_pos[1]);

    // Bucle principal
    while (1) {
        char key = get_keypad_input(); // Asumimos que esta función obtiene la entrada del teclado
        process_keypad(key);
        vTaskDelay(pdMS_TO_TICKS(100)); // Retardo para el rebote
    }
}