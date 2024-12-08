#include <unity.h>
#include <encoder.h>
#include "driver/gpio.h"
#include "esp_timer.h" 
//testeando - Tengo que replicar el código acá si o si porque si no, no me reconoce las funciones... Deberíamos mejorarlo, está atado con alambre
encoder_t encoder1;
encoder_t encoder2;
static volatile int64_t encoder1_last_time = 0;
static volatile int64_t encoder2_last_time = 0;
static volatile direction_t movement_direction = DIRECTION_STOPPED;
static volatile int last_encoder_triggered = 0; // 1 para encoder1, 2 para encoder2
static volatile int64_t last_trigger_time = 0;
void encoder_isr_handler(void *arg) {
    encoder_t *encoder = (encoder_t *)arg;
    int64_t now = esp_timer_get_time();  // Marca de tiempo en microsegundos
    // Incrementar el contador del encoder correspondiente
    encoder->count++;
    // Determinar el orden de las interrupciones
    if (encoder->pin_out == ENCODER1_OUT) {
        if (last_encoder_triggered == 2 && (now - last_trigger_time) < 500000) { 
            // Si el último fue el encoder 2 y la diferencia de tiempo es pequeña
            movement_direction = DIRECTION_FORWARD; 
        }
        printf("Interrupción en encoder 1\n");
        last_encoder_triggered = 1;
    } else if (encoder->pin_out == ENCODER2_OUT) {
        if (last_encoder_triggered == 1 && (now - last_trigger_time) < 500000) { 
            // Si el último fue el encoder 1 y la diferencia de tiempo es pequeña
            movement_direction = DIRECTION_BACKWARD;
            printf("Interrupción en encoder 2\n"); 
        }
        last_encoder_triggered = 2;
    }
    
    // Actualizar el tiempo del último pulso
    last_trigger_time = now;
    encoder->last_pulse_time = now; // Registrar el tiempo del pulso
}
int32_t encoder_get_count(encoder_t *encoder) {
    return encoder->count;
}
void encoder_reset_count(encoder_t *encoder) {
    encoder->count = 0;
}
void encoder_init(encoder_t *encoder, int pin_out) {
    encoder->pin_out = pin_out;
    encoder->count = 0;

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,  // Flanco de bajada
        .mode = GPIO_MODE_INPUT,        // Configurar como entrada
        .pin_bit_mask = (1ULL << pin_out),
        .pull_up_en = GPIO_PULLUP_ENABLE, // Habilitar resistencia pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE // Deshabilitar pull-down
    };
    gpio_config(&io_conf);

    // Instalar servicio de interrupciones si no está instalado
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    // Agregar el controlador de interrupciones
    gpio_isr_handler_add(pin_out, encoder_isr_handler, (void *)encoder);
}

float calcular_disparidad(int contador1, int contador2) {
    int diferencia = abs((contador1/2) - (contador2/2)); // Diferencia absoluta entre ambos contadores
    int maximo = (contador1 > contador2) ? contador1 : contador2; // Mayor de los dos valores

    if (maximo == 0) {
        return 0.0; // Evitar división por cero
    }

    return ((float)diferencia / maximo) * 100.0; // Disparidad en porcentaje
}

float encoder_get_reward(encoder_t *encoder){
    printf("Contador real: %ld \n",encoder_get_count(encoder));
    int32_t real_value= (encoder->count)/2;
    printf("Contador final (divido por 2): %ld \n", real_value);
    float reward = real_value >= 3 ? 1.0 : (float)real_value / 3.0; //Acotado a 5 como max (rendijas de una vuelta)
    return reward;
}



void setUp(void){

}
void tearDown(void){

}
//---------------------TEST UNITARIOS-----------------------------
void test_example(void) {
    TEST_ASSERT_EQUAL(1, 1);  // Ejemplo simple
}
void test_encoder_init(){
    encoder_init(&encoder1, ENCODER1_OUT);
    //encoder_init(&encoder2, ENCODER2_OUT);
    TEST_ASSERT_EQUAL(0,encoder_get_count(&encoder1));
    //TEST_ASSERT_EQUAL(0,encoder_get_count(&encoder2));
}

void test_encoder_get_count(){
    encoder1.count=5;
    encoder2.count=5;
    TEST_ASSERT_EQUAL(5,encoder_get_count(&encoder1));
    //TEST_ASSERT_EQUAL(5,encoder_get_count(&encoder2));
}

void test_encoder_reset_count(){
    encoder_reset_count(&encoder1);
    //encoder_reset_count(&encoder2);
    TEST_ASSERT_EQUAL(0,encoder_get_count(&encoder1));
    //TEST_ASSERT_EQUAL(0,encoder_get_count(&encoder2));
}
void test_calcular_disparidad(){
    int valor1=3;
    int valor2=6;
    int resultado=calcular_disparidad(valor1,valor2);
    TEST_ASSERT_EQUAL(50,resultado);
}
void test_encoder_get_reward(){
    encoder1.count=10;
    float reward=encoder_get_reward(&encoder1);
    TEST_ASSERT_EQUAL_FLOAT(0.5,reward);
}


void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_example);
    RUN_TEST(test_encoder_init);
    RUN_TEST(test_encoder_get_count);
    RUN_TEST(test_encoder_reset_count);
    RUN_TEST(test_calcular_disparidad);
    RUN_TEST(test_encoder_get_reward);
    UNITY_END();
}