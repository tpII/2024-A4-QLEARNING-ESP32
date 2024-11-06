#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#define ROW_NUM 9 // 9 estados (3 posiciones para servo1 y 3 para servo2)
#define ACT_NUM 9  // 3 posiciones para cada servo, combinadas: 3 * 3 = 9
#define MAX_POSITION 90 // máxima posición (grados)
#define MIN_POSITION 0 

typedef struct {
    float Q[ROW_NUM][ACT_NUM]; // tabla Q
    float epsilon; // tasa de exploración
    float alpha; // tasa de aprendizaje
    float gamma; // factor de descuento
} Q_Agent;

Q_Agent agent;
bool crawler_listo = false;  // Indica si el aprendizaje ha finalizado

// definición de funciones
void q_agent_init(Q_Agent *agent);
int q_agent_select_action(Q_Agent *agent, int state);
void q_agent_update(Q_Agent *agent, int state, int action, int reward, int next_state);
void mover_servos(int servo1_position, int servo2_position);
int encoder_signal(); // simula la señal del encoder
void print_q_matrix(Q_Agent *agent); // Nueva función para imprimir la matriz Q
void mover_servos_continuamente(int servo1_initial_position, int servo2_initial_position); // Nueva función para el movimiento continuo de arrastre

void app_main() {
    q_agent_init(&agent);

    int current_state = 0; // defino un estado inicial
    int servo1_position = 0;
    int servo2_position = 0;

    // Proceso de aprendizaje
    while (!crawler_listo) {
        int action = q_agent_select_action(&agent, current_state);
        
        // calcular la nueva posición de los servos basado en la acción seleccionada
        int servo1_new_position = (action % 3) * 45; // acción para el servo 1 (0, 45, 90)
        int servo2_new_position = (action / 3) * 45; // acción para el servo 2 (0, 45, 90)

        // Mover servos solo si la posición nueva es diferente
        if (servo1_position != servo1_new_position || servo2_position != servo2_new_position) {
            mover_servos(servo1_new_position, servo2_new_position);
            servo1_position = servo1_new_position; // Actualiza la posición del servo 1
            servo2_position = servo2_new_position; // Actualiza la posición del servo 2
            
            // Simular la señal del encoder
            int reward = encoder_signal();
            int next_state = (servo1_new_position / 45) + (servo2_new_position / 45) * 3; // nuevo estado basado en las posiciones de los servos

            q_agent_update(&agent, current_state, action, reward, next_state);
            current_state = next_state; // Actualiza el estado actual

            // Imprimir la matriz Q después de cada actualización
            print_q_matrix(&agent);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 1 segundo entre ciclo y ciclo
    }

    // Una vez finalizado el aprendizaje, cambia a modo de arrastre continuo
    printf("Proceso de aprendizaje completado.\n");
    mover_servos_continuamente(servo1_position, servo2_position);  // Inicia el movimiento continuo
}

void q_agent_init(Q_Agent *agent) {
    for (int s = 0; s < ROW_NUM; s++) {
        for (int a = 0; a < ACT_NUM; a++) {
            agent->Q[s][a] = 0;  // inicializa la tabla Q en ceros
        }
    }
    agent->epsilon = 0.1; // exploración
    agent->alpha = 0.1; // tasa de aprendizaje
    agent->gamma = 0.9; // factor de descuento
}

int q_agent_select_action(Q_Agent *agent, int state) {
    if (((float)rand() / RAND_MAX) < agent->epsilon) {
        return rand() % ACT_NUM; // exploración
    } else {
        float max_q = agent->Q[state][0];
        int action = 0;
        for (int a = 1; a < ACT_NUM; a++) {
            if (agent->Q[state][a] > max_q) {
                max_q = agent->Q[state][a];
                action = a;
            }
        }
        return action; // retorna la mejor acción
    }
}

void q_agent_update(Q_Agent *agent, int state, int action, int reward, int next_state) {
    float old_q = agent->Q[state][action];
    float max_q_next = agent->Q[next_state][0];
    
    for (int a = 1; a < ACT_NUM; a++) {
        if (agent->Q[next_state][a] > max_q_next) {
            max_q_next = agent->Q[next_state][a];
        }
    }
    // actualización de Q
    agent->Q[state][action] = old_q + agent->alpha * (reward + agent->gamma * max_q_next - old_q);
}

// Nueva función para imprimir la matriz Q
void print_q_matrix(Q_Agent *agent) {
    printf("Matriz Q:\n");
    for (int s = 0; s < ROW_NUM; s++) {
        for (int a = 0; a < ACT_NUM; a++) {
            printf("%.2f ", agent->Q[s][a]);
        }
        printf("\n");
    }
    printf("\n");
}

void mover_servos(int servo1_position, int servo2_position) {
    // Aquí debería ir el código para mover los servos reales utilizando PWM
    printf("Servo 1 se mueve a: %d grados, Servo 2 se mueve a: %d grados\n", servo1_position, servo2_position);
}

int encoder_signal() {
    // Simula la señal del encoder, retornando un valor de recompensa
    // Aquí debería ir el código para leer la señal del encoder capturando interrupciones
    printf("¡Interrupción del encoder! La rueda se movió.\n");
    return rand() % 2; // Retorna 0 o 1 como recompensa
}

void mover_servos_continuamente(int servo1_initial_position, int servo2_initial_position) {
    int servo1_position = servo1_initial_position;
    int servo2_position = servo2_initial_position;

    while (1) {
        // Paso 1: Mueve el servo 1 a la posición aprendida
        printf("Moviendo Servo 1 a: %d grados\n", servo1_position);

        // Paso 2: Mueve el servo 2 a la posición aprendida
        printf("Moviendo Servo 2 a: %d grados\n", servo2_position);

        // Paso 3: Regresa los servos a la posición original
        printf("Regresando Servo 1 a la posición original\n");
        printf("Regresando Servo 2 a la posición original\n");

        // Paso 4: Repite el ciclo
        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 1 segundo entre ciclos
    }
}