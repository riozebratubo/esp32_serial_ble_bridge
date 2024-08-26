/**
 * Project: ESP32 Ble Serial Bridge
 * Author: riozebratubo
 * Copyright: (c) riozebratubo
 * Project license: CC BY-NC 4.0
 **/

#include "uart.h"

#include <string.h>

#include "esp_log.h"
#include "esp_gatts_api.h"

#include "freertos/task.h"

#include "settings.h"
#include "commands.h"

static char* log_tag = "SERIALBLE_UART";

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

extern app_settings_t app_settings;
extern bool can_transmit;
extern esp_gatt_if_t last_gatts_if;
extern uint16_t last_conn_id;
extern uint16_t last_char_handle;
extern uint16_t last_mtu;

extern TaskHandle_t xDataUartServiceHandle;
extern TaskHandle_t xConfigUartServiceHandle;

#define COMMANDS_MODULE_BUFFER_SIZE 2048
extern char commands_buffer[COMMANDS_MODULE_BUFFER_SIZE];
extern uint32_t commands_position;

static QueueHandle_t uart1_queue;
static QueueHandle_t uart0_queue;

uint8_t* uart_data;
uint8_t* uart_config_data_tx;
uint8_t* uart_config_data_rx;

uart_config_t uart_data_config = {
    .baud_rate =  115200,
    .data_bits =  UART_DATA_8_BITS,
    .parity =     UART_PARITY_DISABLE,
    .stop_bits =  UART_STOP_BITS_1,
    .flow_ctrl =  UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB,
};

uart_config_t uart_config_config = {
    .baud_rate =  115200,
    .data_bits =  UART_DATA_8_BITS,
    .parity =     UART_PARITY_DISABLE,
    .stop_bits =  UART_STOP_BITS_1,
    .flow_ctrl =  UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};

void uart_module_init() {
    uart_data = (uint8_t*) malloc(app_settings.uart_buffer_size * sizeof(uint8_t));
    if (!uart_data) return;

    uart_data_config.baud_rate = app_settings.uart_baud_rate;

    uart_driver_install(UART_NUM_1, app_settings.uart_buffer_size, app_settings.uart_buffer_size, 50, &uart1_queue, 0);
    uart_param_config(UART_NUM_1, &uart_data_config);
    uart_set_mode(UART_NUM_1, UART_MODE_UART);
    uart_set_pin(UART_NUM_1, app_settings.uart_pin_tx, app_settings.uart_pin_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_config_data_tx = (uint8_t*) malloc(app_settings.uart_buffer_size * sizeof(uint8_t));
    if (!uart_config_data_tx) return;

    uart_config_data_rx = (uint8_t*) malloc(app_settings.uart_buffer_size * sizeof(uint8_t));
    if (!uart_config_data_rx) return;

    uart_driver_install(UART_NUM_0, app_settings.uart_buffer_size, app_settings.uart_buffer_size, 50, &uart0_queue, 0);
    uart_param_config(UART_NUM_0, &uart_config_config);
    uart_set_mode(UART_NUM_0, UART_MODE_UART);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void uart_module_vtask_uart_config_handle(void* pvParameters) {
    uart_event_t event;
    while (1) {
        if (xQueueReceive(uart0_queue, (void*)&event, (TickType_t)portMAX_DELAY)) {
            bzero(uart_config_data_rx, app_settings.uart_buffer_size);
            switch (event.type) {
                case UART_DATA:
                    int read_len = uart_read_bytes(UART_NUM_0, uart_config_data_rx, app_settings.uart_buffer_size, 100 / portTICK_PERIOD_MS);
                    memcpy(commands_buffer + commands_position, uart_config_data_rx, read_len);
                    commands_position += read_len;
                    commands_buffer[commands_position] = '\0';
                    memset(uart_config_data_rx, '\0', read_len + 1);
                    if (strchr(commands_buffer + commands_position - read_len, '\r')) {
                        commands_module_process_command();
                    }
                    break;
                default:
            }
        }
    }
}

void uart_module_vtask_uart_read(void* pvParameters) {
    while (1) {
        int read_len = uart_read_bytes(UART_NUM_1, uart_data, (app_settings.uart_buffer_size - 1), 1 / portTICK_PERIOD_MS);
        if (read_len > 0) {
            if (can_transmit) {
                ESP_LOGI(log_tag, "Sending data to BLE: [mtu: %d, read size: %d, data: %.*s]", last_mtu, read_len, read_len, uart_data);
                for (int i = 0; i < read_len; i += last_mtu) {
                  int len = min(last_mtu, read_len - i);
                  if (can_transmit == false) break;
                  int ret = esp_ble_gatts_send_indicate(last_gatts_if, last_conn_id, last_char_handle, len, uart_data + i, false);
                  if (ret == ESP_OK) ESP_LOGI(log_tag, "Send chunk ok [size: %d]", len);
                  else ESP_LOGI(log_tag, "Send chunk FAILED [size: %d]", len);
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void uart_module_start_task() {
    static uint8_t ucParameterToPass;
    xTaskCreatePinnedToCore(uart_module_vtask_uart_read, "UART_HANDLE_DATA_TASK", 4096, &ucParameterToPass, tskIDLE_PRIORITY, &xDataUartServiceHandle, tskNO_AFFINITY);
    configASSERT(xDataUartServiceHandle);
    xTaskCreate(uart_module_vtask_uart_config_handle, "UART_CONFIG_HANDLE_TASK", 10240, NULL, 12, &xConfigUartServiceHandle);
    configASSERT(xConfigUartServiceHandle);
}

int uart_module_write_bytes(uart_port_t uart_num, const void *src, size_t size) {
    return uart_write_bytes(uart_num, src, size);
}
