#include <unity.h>
#include <encoder.h>
void setUp(void){

}
void tearDown(void){

}

void test_example(void) {
    TEST_ASSERT_EQUAL(1, 1);  // Ejemplo simple
}
void test_encoder_init(){
    encoder_init(&encoder1, ENCODER1_OUT);
    encoder_init(&encoder2, ENCODER2_OUT);
    TEST_ASSERT_EQUAL(encoder1.count, 0);
    TEST_ASSERT_EQUAL(encoder2.count, 0);
}

void test_encoder_get_count(){
    encoder1.count=encoder1.count+5;
    encoder2.count=encoder2.count+5;
    TEST_ASSERT_EQUAL(encoder_get_count(&encoder1),5);
    TEST_ASSERT_EQUAL(encoder_get_count(&encoder2),5);
}

void test_encoder_reset_count(){
    encoder_reset_count(&encoder1);
    encoder_reset_count(&encoder2);
    TEST_ASSERT_EQUAL(encoder_get_count(&encoder1),0);
    TEST_ASSERT_EQUAL(encoder_get_count(&encoder2),0);
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_example);
    RUN_TEST(test_encoder_init);
    RUN_TEST(test_encoder_get_count);
    RUN_TEST(test_encoder_reset_count);
    UNITY_END();
}