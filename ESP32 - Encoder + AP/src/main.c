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

// Definir los pines de salida de los encoders
#define ENCODER1_OUT 06
#define ENCODER2_OUT 07

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

// Configuración del main
void app_main() {
    uart_set_baudrate(UART_NUM_0, 115200);
    nvs_flash_init();  // Inicializa NVS
    wifi_init_softap();  // Inicia AP


    // Inicializar los encoders
    encoder_init(&encoder1, ENCODER1_OUT);
    encoder_init(&encoder2, ENCODER2_OUT);

    while (1) {
        int32_t count1 = encoder_get_count(&encoder1);
        int32_t count2 = encoder_get_count(&encoder2);
        printf("Pulsos Encoder 1: %ld, Pulsos Encoder 2: %ld \n", count1, count2);

        // Enviar datos de ambos encoders mediante POST cada 5 segundos
        char post_data[100];
        snprintf(post_data, sizeof(post_data), 
                 "{\"encoder1\": %ld , \"encoder2\": %ld }", count1, count2);
        //http_post("http://192.168.4.2:8080/post", post_data);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
