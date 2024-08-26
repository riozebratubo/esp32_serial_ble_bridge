/**
 * Project: ESP32 Ble Serial Bridge
 * Author: riozebratubo
 * Copyright: (c) riozebratubo
 * Project license: CC BY-NC 4.0
 **/

#include <stdint.h>

typedef struct app_settings_t {
    /* ble */
    char complete_service_uuid[37];
    char complete_characteristic_uuid[37];
    char device_name[15];
    uint8_t manufacturer_bytes[4];
    uint32_t desired_ble_mtu;
    char force_mac_address[18];

    /* uart */
    uint32_t uart_pin_tx;
    uint32_t uart_pin_rx;
    uint32_t uart_buffer_size;
    uint32_t uart_baud_rate;
} app_settings_t;

typedef struct app_settings_version_t {
    int32_t version;
} app_settings_version_t;

#define CURRENT_SETTINGS_VERSION 1

uint8_t settings_data_valid(app_settings_t* app_settings);
uint8_t settings_init();
void settings_log(app_settings_t* app_settings);
void settings_user_list(app_settings_t* app_settings);
uint8_t settings_save(app_settings_t* app_settings);
uint8_t settings_load(app_settings_t* app_settings);
uint8_t settings_remove_files();
