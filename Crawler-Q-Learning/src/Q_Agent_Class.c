// #include "Q_Agent_Class.h"
// #include <math.h>

// void q_agent_init(Q_Agent *agent) {
//     for (int s = 0; s < ROW_NUM; s++) {
//         for (int a = 0; a < ACT_NUM; a++) {
//             agent->Q[s][a] = 0;  // Inicializa la tabla Q en ceros
//         }
//     }
// }

// int q_agent_select_action(Q_Agent *agent, int state) {
//     if (((float)rand() / RAND_MAX) < agent->epsilon) {
//         // Selección aleatoria para exploración
//         return rand() % ACT_NUM;
//     } else {
//         // Selección de la mejor acción conocida
//         float max_q = agent->Q[state][0];
//         int action = 0;
//         for (int a = 1; a < ACT_NUM; a++) {
//             if (agent->Q[state][a] > max_q) {
//                 max_q = agent->Q[state][a];
//                 action = a;
//             }
//         }
//         return action;
//     }
// }

// void q_agent_update(Q_Agent *agent, int state, int action, int reward, int next_state) {
//     float old_q = agent->Q[state][action];
//     float max_q_next = agent->Q[next_state][0];
    
//     // Busca el máximo Q para el siguiente estado
//     for (int a = 1; a < ACT_NUM; a++) {
//         if (agent->Q[next_state][a] > max_q_next) {
//             max_q_next = agent->Q[next_state][a];
//         }
//     }
//     // Actualización de Q-valor
//     agent->Q[state][action] = old_q + agent->alpha * (reward + agent->gamma * max_q_next - old_q);
// }
