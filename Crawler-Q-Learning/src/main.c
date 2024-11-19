#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "servo.c"

#define SERVO_NUM 2 // Dos servos
#define ROW_NUM 9 // 9 estados (3 posiciones para servo1 y 3 para servo2)
#define ACT_NUM 5 // 5 acciones por cada servo: adelante, atrás, sin movimiento, y dos más si se definen
#define MAX_POSITION 90 // máxima posición (grados)
#define MIN_POSITION 0 

typedef struct {
    float Q[SERVO_NUM][ROW_NUM][ACT_NUM]; // matriz Q tridimensional
    float R[SERVO_NUM][ROW_NUM][ACT_NUM]; // matriz R tridimensional (recompensas)
    float epsilon; // tasa de exploración
    float alpha; // tasa de aprendizaje
    float gamma; // factor de descuento
} Q_Agent;

Q_Agent agent;
bool crawler_listo = false; // Indica si el aprendizaje ha finalizado

// definición de funciones
void q_agent_init(Q_Agent *agent);
int q_agent_select_action(Q_Agent *agent, int servo, int state);
void q_agent_update(Q_Agent *agent, int servo, int state, int action, int reward, int next_state);
void mover_servos(int servo1_position, int servo2_position);
int encoder_signal(); // simula la señal del encoder
void print_q_matrix(Q_Agent *agent); // Nueva función para imprimir la matriz Q
void mover_servos_continuamente(int servo1_initial_position, int servo2_initial_position); // Nueva función para el movimiento continuo de arrastre

void app_main() {
    q_agent_init(&agent);

    int current_state = 0; // estado inicial
    int servo1_position = 0;
    int servo2_position = 0;
    int cont = 0;

    // Proceso de aprendizaje
    while (!crawler_listo && (cont < 100)) {
        // Seleccionar acción para cada servo
        int servo1_action = q_agent_select_action(&agent, 0, current_state);
        int servo2_action = q_agent_select_action(&agent, 1, current_state);

        // Calcular nuevas posiciones basadas en las acciones seleccionadas
        int servo1_new_position = servo1_position + (servo1_action == 0 ? 45 : (servo1_action == 1 ? -45 : 0));
        int servo2_new_position = servo2_position + (servo2_action == 0 ? 45 : (servo2_action == 1 ? -45 : 0));

        // Restringir posiciones a los límites
        servo1_new_position = servo1_new_position > MAX_POSITION ? MAX_POSITION : (servo1_new_position < MIN_POSITION ? MIN_POSITION : servo1_new_position);
        servo2_new_position = servo2_new_position > MAX_POSITION ? MAX_POSITION : (servo2_new_position < MIN_POSITION ? MIN_POSITION : servo2_new_position);

        // Calcular nuevo estado
        int next_state = (servo1_new_position / 45) + (servo2_new_position / 45) * 3;

        // Simular recompensas del encoder
        int reward1 = encoder_signal();
        int reward2 = encoder_signal();

        // Mover los servos
        mover_servos(servo1_new_position, servo2_new_position);

        // Actualizar las matrices Q para ambos servos
        q_agent_update(&agent, 0, current_state, servo1_action, reward1, next_state);
        q_agent_update(&agent, 1, current_state, servo2_action, reward2, next_state);

        // Actualizar estado y posiciones
        current_state = next_state;
        servo1_position = servo1_new_position;
        servo2_position = servo2_new_position;

        // Imprimir matriz Q
        print_q_matrix(&agent);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 1 segundo entre ciclos
        cont++;
    }

    // Una vez finalizado el aprendizaje, cambiar a modo de arrastre continuo
    printf("Proceso de aprendizaje completado.\n");
    mover_servos_continuamente(servo1_position, servo2_position);
}

void q_agent_init(Q_Agent *agent) {
    for (int servo = 0; servo < SERVO_NUM; servo++) {
        for (int s = 0; s < ROW_NUM; s++) {
            for (int a = 0; a < ACT_NUM; a++) {
                agent->Q[servo][s][a] = 0; // Inicializa la matriz Q en ceros
                agent->R[servo][s][a] = 0; // Inicializa la matriz R en ceros
            }
        }
    }
    agent->epsilon = 0.1; // exploración
    agent->alpha = 0.1; // tasa de aprendizaje
    agent->gamma = 0.9; // factor de descuento
}

int q_agent_select_action(Q_Agent *agent, int servo, int state) {
    if (((float)rand() / RAND_MAX) < agent->epsilon) {
        return rand() % ACT_NUM; // exploración
    } else {
        float max_q = agent->Q[servo][state][0];
        int action = 0;
        for (int a = 1; a < ACT_NUM; a++) {
            if (agent->Q[servo][state][a] > max_q) {
                max_q = agent->Q[servo][state][a];
                action = a;
            }
        }
        return action; // Retorna la mejor acción (del vector de acciones 0-5)
    }
}

void q_agent_update(Q_Agent *agent, int servo, int state, int action, int reward, int next_state) {
    float old_q = agent->Q[servo][state][action];
    float max_q_next = agent->Q[servo][next_state][0];

    for (int a = 1; a < ACT_NUM; a++) {
        if (agent->Q[servo][next_state][a] > max_q_next) {
            max_q_next = agent->Q[servo][next_state][a];
        }
    }
    // Actualización de Q
    agent->Q[servo][state][action] = old_q + agent->alpha * (reward + agent->gamma * max_q_next - old_q);
}

void print_q_matrix(Q_Agent *agent) {
    for (int servo = 0; servo < SERVO_NUM; servo++) {
        printf("Matriz Q para Servo %d:\n", servo + 1);
        for (int s = 0; s < ROW_NUM; s++) {
            for (int a = 0; a < ACT_NUM; a++) {
                printf("%.2f ", agent->Q[servo][s][a]);
            }
            printf("\n");
        }
        printf("\n");
    }
}

void mover_servos(int servo1_position, int servo2_position) {
    // Aquí debería ir el código para mover los servos reales utilizando PWM
    printf("Servo 1: %d grados, Servo 2: %d grados\n", servo1_position, servo2_position);
}

int encoder_signal() {
    // Simula una recompensa aleatoria
    return rand() % 2; // Retorna 0 o 1
}

void mover_servos_continuamente(int servo1_initial_position, int servo2_initial_position) {
    while (1) {
        printf("Movimiento continuo: Servo 1 a %d, Servo 2 a %d\n", servo1_initial_position, servo2_initial_position);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
