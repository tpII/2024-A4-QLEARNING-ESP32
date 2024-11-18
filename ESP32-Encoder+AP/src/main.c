#include <stdio.h>
#include <string.h>  // Para usar strlen() y snprintf()
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "encoder.h"  // Incluir el archivo del encoder
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "driver/uart.h"
#include "esp_task_wdt.h"



// Configuración del AP
#define AP_SSID "MiESP32_AP"
#define AP_PASSWORD "123456789"
#define MAX_STA_CONN 4

encoder_t encoder1, encoder2;  // Instancias de los dos encoders

// Configura el modo AP del ESP32
void wifi_init_softap() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .password = AP_PASSWORD,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };

    if (strlen(AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_start();

    printf("Punto de acceso inicializado: SSID:%s\n", AP_SSID);
}

// Maneja eventos HTTP
esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    if (evt->event_id == HTTP_EVENT_ERROR) {
        printf("Error en la solicitud HTTP\n");
    }
    return ESP_OK;
}

// Enviar una solicitud HTTP POST
void http_post(const char *url, const char *post_data) {
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        printf("POST enviado con éxito, código de respuesta: %d\n", 
               esp_http_client_get_status_code(client));
    } else {
        printf("Error en el POST: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void enviarDatosMatriz(const char *data){
    http_post("http://192.168.4.2:8080/recibir_dato", data);
}


// Configuración del main
void app_main() {
    uart_set_baudrate(UART_NUM_0, 115200);
    nvs_flash_init();  // Inicializa NVS
    wifi_init_softap();  // Inicia AP

    esp_task_wdt_deinit();  // Desactiva el watchdog para las tareas TESTEANDO


    // Inicializar los encoders
    encoder_init(&encoder1, ENCODER1_OUT);
    encoder_init(&encoder2, ENCODER2_OUT);
    xTaskCreate(tarea_verificar_variable,      // Función de la tarea
                "VerificarVariableTask",       // Nombre de la tarea
                2048,                          // Tamaño del stack
                NULL,                          // Parámetro de entrada
                1,                             // Prioridad
                NULL);
    vTaskDelay(pdMS_TO_TICKS(5000));

    //Provisorio, dummy. Ajustar para que sea la matriz de verdad
    int matriz[9][9] = {//Dummy. Enviar la que en verdad es
        {1,  2,  3,  4,  5,  6,  7,  8,  9},
        {10, 11, 12, 13, 14, 15, 16, 17, 18},
        {19, 20, 21, 22, 23, 24, 25, 26, 27},
        {28, 29, 30, 31, 32, 33, 34, 35, 36},
        {37, 38, 39, 40, 41, 42, 43, 44, 45},
        {46, 47, 48, 49, 50, 51, 52, 53, 54},
        {55, 56, 57, 58, 59, 60, 61, 62, 63},
        {64, 65, 66, 67, 68, 69, 70, 71, 72},
        {73, 74, 75, 76, 77, 78, 79, 80, 81}
        };

    while (1) {
        int32_t count1 = encoder_get_count(&encoder1);
        int32_t count2 = encoder_get_count(&encoder2);
        printf("Pulsos Encoder 1: %ld, Pulsos Encoder 2: %ld \n", count1, count2);

        direction_t dir = get_movement_direction();
        if (dir == DIRECTION_FORWARD) {
            printf("Hacia adelante\n");
        } else if (dir == DIRECTION_BACKWARD) {
            printf("Hacia atrás\n");
        } else {
            printf("Detenido\n");
        }
        
        char post_data[100];
        snprintf(post_data, sizeof(post_data), 
                 "{\"encoder1\": %ld , \"encoder2\": %ld }", count1, count2);
        //9x9. Función para enviar datos al servidor
        
        enviarDatosMatriz(matriz);
        //
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}