/**
 * Project: ESP32 Ble Serial Bridge
 * Author: riozebratubo
 * Copyright: (c) riozebratubo
 * Project license: CC BY-NC 4.0
 **/

#include "utils.h"
#include "settings.h"
#include "ble.h"
#include "commands.h"
#include "uart.h"

// *** default settings - change here ***

app_settings_t app_settings = {
    .complete_service_uuid = "0000FFE0-0000-1000-8000-00805F9B34FB",
    .complete_characteristic_uuid = "0000FFE1-0000-1000-8000-00805F9B34FB",
    .device_name = "ESPSERIALBLE",
    .manufacturer_bytes = {0x12, 0x23, 0x45, 0x56},
    .desired_ble_mtu = 400,
    .force_mac_address = "",
    .uart_pin_tx = 17,
    .uart_pin_rx = 18,
    .uart_buffer_size = 2048,
    .uart_baud_rate = 115200
};

// *** default settings - change here ***

TaskHandle_t xDataUartServiceHandle = NULL;
TaskHandle_t xConfigUartServiceHandle = NULL;

static char* log_tag = "SERIALBLE";

void app_main(void) {
    if (settings_init()) {
        if (!settings_load(&app_settings)) {
            ESP_LOGE(log_tag, "Warning: could not read app settings file. Using default app settings.");
        }
    }
    else {
        ESP_LOGE(log_tag, "Warning: could not initialize settings file service. Using default app settings.");
    }

    settings_log(&app_settings);

    ble_init_and_start();

    commands_module_init();
    uart_module_init();
    uart_module_start_task();

    return;
}
