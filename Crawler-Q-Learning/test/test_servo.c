#include "unity.h"
#include "servo.h"

// Define the current_pos array to store servo positions
extern int current_pos[2];

void setUp(void) {
    // Configurar los servos antes de cada prueba
    init_servo();
}

void tearDown(void) {
    // Limpiar después de cada prueba si es necesario
}

void test_set_servo_pulse(void) {
    // Prueba para verificar la configuración del pulso del servo
    process_move_shoulder(45);
    set_servo_pulse(LEDC_SHOULDER_CHANNEL, SHOULDER_MID_PULSE);
    TEST_ASSERT_EQUAL(SHOULDER_MID_PULSE, current_pos[0]);


    process_move_elbow(45);
    set_servo_pulse(LEDC_ELBOW_CHANNEL, ELBOW_MID_PULSE);
    TEST_ASSERT_EQUAL(ELBOW_MID_PULSE, current_pos[1]);
}

void test_process_move_shoulder(void) {
    // Prueba para verificar el movimiento del hombro
    process_move_shoulder(0);
    TEST_ASSERT_EQUAL(SHOULDER_MIN_PULSE, current_pos[0]);

    process_move_shoulder(45);
    TEST_ASSERT_EQUAL(SHOULDER_MID_PULSE, current_pos[0]);

    process_move_shoulder(90);
    TEST_ASSERT_EQUAL(SHOULDER_MAX_PULSE, current_pos[0]);
}

void test_process_move_elbow(void) {
    // Prueba para verificar el movimiento del codo
    process_move_elbow(0);
    TEST_ASSERT_EQUAL(ELBOW_MIN_PULSE, current_pos[1]);

    process_move_elbow(45);
    TEST_ASSERT_EQUAL(ELBOW_MID_PULSE, current_pos[1]);

    process_move_elbow(90);
    TEST_ASSERT_EQUAL(ELBOW_MAX_PULSE, current_pos[1]);
}

void test_set_pos(void) {
    // Prueba para verificar la configuración de la posición
    set_pos(SHOULDER_MID_PULSE, ELBOW_MID_PULSE);
    TEST_ASSERT_EQUAL(SHOULDER_MID_PULSE, current_pos[0]);
    TEST_ASSERT_EQUAL(ELBOW_MID_PULSE, current_pos[1]);
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_set_servo_pulse);
    RUN_TEST(test_process_move_shoulder);
    RUN_TEST(test_process_move_elbow);
    RUN_TEST(test_set_pos);
    return UNITY_END();
}

int app_main(void) {
    return runUnityTests();
}