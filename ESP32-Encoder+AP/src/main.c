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
#include "cJSON.h" // Librería para procesar JSON (incluida en ESP-IDF)
#define TAG "HTTP_GET"

//AGREGAR A CODIGO FINAL ------------------------------------------------------------
char *response_data = NULL;
size_t response_data_size = 0;
int estadoCrawler=-1; //-1 Detenido, 1 Empezado(haciendo algo)
int estadoAprendiendoEjecutando=-1; //-1 Detenido, 0 Aprendiendo, 1 Ejecutando


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
        //printf("Error en la solicitud HTTP\n");
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
        //printf("Error en el POST: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

// Función para manejar la respuesta del GET
//NUEVO HTTP, AGREGAR A CODIGO FINAL -------------------------------------------------------------------
int process_get_response(const char *response) {
    // Parsear el JSON
    cJSON *json = cJSON_Parse(response);
    if (json == NULL) {
        printf("Error al parsear la respuesta JSON\n");
        return 0;
    }
    int respuesta=0;

    // Extraer el valor de "start"
    cJSON *start_item = cJSON_GetObjectItem(json, "start");
    if (cJSON_IsBool(start_item)) {
        bool start = cJSON_IsTrue(start_item);
        printf("Estado de start: %s\n", start ? "true" : "false");
        if(start){
            respuesta=1;
        }
    } else {
        printf("No se encontró el campo 'start' o no es un booleano\n");
    }

    // Liberar la memoria del objeto JSON
    cJSON_Delete(json);
    return respuesta;
}

// Método HTTP GET -- LO ESTOY CAMBIANDO
/*
int http_get(const char *url) {
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler, // Maneja eventos HTTP
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    int estado=0;
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        printf("GET exitoso, código de respuesta: %d\n", esp_http_client_get_status_code(client));

        //DEBUGEANDO
        // Lee el cuerpo de la respuesta
        char response_buffer[256];
        int content_length = esp_http_client_read(client, response_buffer, sizeof(response_buffer) - 1);
        printf("Content length: %d \n",content_length);
        char *content_type = NULL; // Variable para almacenar el valor del encabezado
        esp_err_t ret = esp_http_client_get_header(client, "Content-Type", &content_type);
        if (ret == ESP_OK && content_type != NULL) {
            printf("Content-Type: %s\n", content_type);
        } else {
            printf("No se pudo obtener Content-Type o no está presente.\n");
        }

        printf("Responde Buffer: %s \n", response_buffer);
        if (content_length > 0) {
            response_buffer[content_length] = '\0'; // Termina el string
            printf("Respuesta del servidor: %s\n", response_buffer);

            // Procesar la respuesta JSON
            estado=process_get_response(response_buffer);
        }
    } else {
        printf("Error en el GET: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return estado;
}
*/


//NUEVO HTTP, AGREGAR A CODIGO FINAL -------------------------------------------------------------------
// Método HTTP GET mejorado
// Método HTTP GET con soporte JSON
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA, longitud de datos: %d\n", evt->data_len);
        // Redimensionar el buffer para almacenar los nuevos datos
        response_data = realloc(response_data, response_data_size + evt->data_len + 1); // +1 para el terminador nulo
        if (response_data == NULL)
        {
            printf("Error al redimensionar el buffer de respuesta.\n");
            return ESP_FAIL;
        }

        // Copiar los nuevos datos al buffer
        memcpy(response_data + response_data_size, evt->data, evt->data_len);
        response_data_size += evt->data_len;
        response_data[response_data_size] = '\0'; // Asegurarse de que sea una cadena terminada en nulo
        break;

    default:
        break;
    }
    return ESP_OK;
}
int http_get(const char *url)
{
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = client_event_get_handler, // Usar el manejador de eventos
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    int estado = estadoCrawler; // Por defecto, asumimos error

    // Realizar la solicitud HTTP GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        printf("GET exitoso, código de respuesta: %d\n", status_code);

        if (response_data != NULL)
        {
            printf("Respuesta completa del servidor: %s\n", response_data);

            // Analizar el contenido JSON usando cJSON
            cJSON *json_response = cJSON_Parse(response_data);
            if (json_response == NULL)
            {
                printf("Error al analizar JSON.\n");
            }
            else
            {
                // Extraer valor del campo "start"
                cJSON *start = cJSON_GetObjectItemCaseSensitive(json_response, "start");
                if (cJSON_IsBool(start))
                {
                    estado = cJSON_IsTrue(start) ? 1 : -1; // 1 si es `true`, 0 si es `false`
                    printf("Valor de 'start': %s\n", estado ? "true" : "false");
                }
                else
                {
                    printf("El campo 'start' no es booleano o no existe.\n");
                }
                cJSON_Delete(json_response); // Libera la memoria del objeto JSON
            }

            free(response_data); // Libera el buffer de respuesta
            response_data = NULL;
            response_data_size = 0;
        }
        else
        {
            printf("No se recibió respuesta del servidor.\n");
        }
    }
    else
    {
        printf("Error en el GET: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return estado;
}

void enviarDatosMatriz(int matriz[9][9]) {
    char buffer[1024]; // Buffer para el string JSON
    int offset = 0;    // Offset para ir escribiendo en el buffer

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "{ \"matriz\": [");

    for (int i = 0; i < 9; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "[");
        for (int j = 0; j < 9; j++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d", matriz[i][j]);
            if (j < 8) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",");
            }
        }
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "]");
        if (i < 8) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",");
        }
    }

    snprintf(buffer + offset, sizeof(buffer) - offset, "]}");

    //printf("Datos JSON generados: %s\n", buffer);

    // Llamar a http_post con el JSON generado
    http_post("http://192.168.4.2:8000/api/recibir_dato/", buffer);
}

int obtenerEstadoCrawler(){
    printf("Solicitando estado de la variable start...\n");
    int estado=http_get("http://192.168.4.2:8000/get_start_state/"); //0 - Detener ...... 1 - Empezar
    return estado;
}

//AGREGAR A CODIGO FINAL ----------------------------------------------------
void enviarEstadoCrawler(){
    char buffer[128];
    int length = snprintf(buffer, sizeof(buffer), "{ \"estado\": %d }", estadoAprendiendoEjecutando);
    if (length < 0 || length >= sizeof(buffer)) {
        printf("Error al generar el JSON.\n");
        return;
    }
    // Imprimir el JSON
    printf("JSON generado:\n%s\n", buffer);
    http_post("http://192.168.4.2:8000/api/recibir_estado/", buffer);
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
    encoders_params_t encoders = {
        .encoder1 = &encoder1,
        .encoder2 = &encoder2,
    };
    xTaskCreate(tarea_verificar_variable,      // Función de la tarea
                "VerificarVariableTask",       // Nombre de la tarea
                2048,                          // Tamaño del stack
                (void *)&encoders,             // Parámetro de entrada
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
            printf("Direccion: Hacia adelante\n");
        } else if (dir == DIRECTION_BACKWARD) {
            printf("Direccion: Hacia atrás\n");
        } else {
            printf("Direccion: Detenido\n");
        }
        
        char post_data[100];
        snprintf(post_data, sizeof(post_data), 
                 "{\"encoder1\": %ld , \"encoder2\": %ld }", count1, count2);
        //9x9. Función para enviar datos al servidor
        
        //Obtiene recompensa encoders
        printf("Reward: %f\n", get_reward(&encoder1,&encoder2));

        
        //
        vTaskDelay(pdMS_TO_TICKS(1000));
        int estadoCrawler=obtenerEstadoCrawler();
        if(estadoCrawler==-1){
            printf("Estado: Crawler detenido\n");
        }
        else{
            printf("Estado: Crawler comenzado\n");
        }
        printf("Enviando datos de matriz...\n");
        enviarDatosMatriz(matriz);
        printf("Enviando estado de crawler..\n");
        enviarEstadoCrawler();

        //No meter, testing
        if(estadoAprendiendoEjecutando==-1){
            estadoAprendiendoEjecutando=0;
        }
        else if(estadoAprendiendoEjecutando==0){
            estadoAprendiendoEjecutando=1;
        }
        else if(estadoAprendiendoEjecutando==1){
            estadoAprendiendoEjecutando=-1;
        }
    }
    return;
}