/**
 * Project: ESP32 Ble Serial Bridge
 * Author: riozebratubo
 * Copyright: (c) riozebratubo
 * Project license: CC BY-NC 4.0
 **/

#include "commands.h"

#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_system.h"

#include "settings.h"
#include "utils.h"

extern app_settings_t app_settings;

#define COMMANDS_MODULE_BUFFER_SIZE 4096
char commands_buffer[COMMANDS_MODULE_BUFFER_SIZE];
uint32_t commands_position;

static char* log_tag = "SERIALBLE_COMMANDS";

void commands_module_init() {
    memset(commands_buffer, '\0', COMMANDS_MODULE_BUFFER_SIZE);
    commands_position = 0;
}

uint8_t commands_module_process_one_command(int command_length) {
    if (commands_position >= 4 && strncmp(commands_buffer, "set ", 4) == 0) {
        if (command_length < 5) { return 0; }
        size_t setting_position = 4;
        while (commands_buffer[setting_position] == ' ' && setting_position + 1 <= command_length) setting_position++;
        size_t setting_end_position = setting_position;
        while (commands_buffer[setting_end_position] != ' ' && commands_buffer[setting_end_position] != '=' && setting_end_position <= command_length) setting_end_position++;
        setting_end_position--;

        if ((int) setting_end_position - (int) setting_position >= 0) {
            size_t equals_position = setting_end_position + 1;
            while (commands_buffer[equals_position] == ' ' && equals_position + 1 <= command_length) equals_position++;
            size_t value_position = equals_position + 1;
            while (commands_buffer[value_position] == ' ' && value_position + 1 <= command_length) value_position++;
            value_position++;
            size_t end_quotes_position = command_length - 1;
            while (commands_buffer[end_quotes_position] == ' ' && end_quotes_position + 1 > value_position) end_quotes_position--;
            size_t value_end_position = end_quotes_position - 1;

            if ((int) value_end_position - (int) value_position + 1 >= 0 && commands_buffer[value_position - 1] == '"' && commands_buffer[value_end_position + 1] == '"') {
                char* setting_name = "complete_service_uuid";
                size_t setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    if (value_end_position - value_position + 1 == 36 && uuid_char_array_is_valid(commands_buffer + value_position)) {
                        ESP_LOGI(log_tag, "Valid uuid, setting it to service: %.*s", value_end_position - value_position + 1, commands_buffer + value_position);
                        memcpy(&app_settings.complete_service_uuid, commands_buffer + value_position, 36);
                        app_settings.complete_service_uuid[36] = '\0';
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid uuid: %.*s", value_end_position - value_position + 1, commands_buffer + value_position);
                    }
                }

                setting_name = "complete_characteristic_uuid";
                setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    if (value_end_position - value_position + 1 == 36 && uuid_char_array_is_valid(commands_buffer + value_position)) {
                        ESP_LOGI(log_tag, "Valid uuid, setting it to characteristic: %.*s", value_end_position - value_position + 1, commands_buffer + value_position);
                        memcpy(&app_settings.complete_characteristic_uuid, commands_buffer + value_position, 36);
                        app_settings.complete_characteristic_uuid[36] = '\0';
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid uuid: %.*s", value_end_position - value_position + 1, commands_buffer + value_position);
                    }
                }

                setting_name = "device_name";
                setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    size_t value_length = value_end_position - value_position + 1;
                    if (value_length <= sizeof(app_settings.device_name) - 1) {
                        ESP_LOGI(log_tag, "Valid device name, setting it: %.*s", value_end_position - value_position + 1, commands_buffer + value_position);
                        memcpy(&app_settings.device_name, commands_buffer + value_position, value_length);
                        app_settings.device_name[14] = '\0';
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid device name, check size: %.*s", value_end_position - value_position + 1, commands_buffer + value_position);
                    }
                }

                setting_name = "desired_ble_mtu";
                setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    char *conversion_end;
                    int32_t int_value = strtol(commands_buffer + value_position, &conversion_end, 10);
                    if (conversion_end == commands_buffer + value_end_position + 1 && int_value > 0 && int_value < 10000) {
                        ESP_LOGI(log_tag, "Valid ble mtu, setting it: %ld", int_value);
                        app_settings.desired_ble_mtu = (uint32_t) int_value;
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid ble mtu, check limits: %ld", int_value);
                    }
                }

                setting_name = "force_mac_address";
                setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    if (
                        (value_end_position - value_position + 1 == 17 && 
                        mac_address_char_array_is_valid(commands_buffer + value_position))
                        || 
                        (value_end_position - value_position + 1 == 0)
                        ) {
                        ESP_LOGI(log_tag, "Valid mac address, setting it: %.*s", value_end_position - value_position + 1, commands_buffer + value_position);
                        if (value_end_position - value_position + 1 == 17) {
                            memcpy(&app_settings.force_mac_address, commands_buffer + value_position, 17);
                            app_settings.force_mac_address[17] = '\0';
                        }
                        else if (value_end_position - value_position + 1 == 0) {
                            memset(&app_settings.force_mac_address, '\0', 17);
                        }
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid mac address, check format and size: %.*s", value_end_position - value_position + 1, commands_buffer + value_position);
                    }
                }

                setting_name = "uart_pin_tx";
                setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    char *conversion_end;
                    int32_t int_value = strtol(commands_buffer + value_position, &conversion_end, 10);
                    if (conversion_end == commands_buffer + value_end_position + 1 && int_value >= 0 && int_value < 60) {
                        ESP_LOGI(log_tag, "Valid tx pin, setting it: %ld", int_value);
                        app_settings.uart_pin_tx = (uint32_t) int_value;
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid tx pin, check limits: %ld", int_value);
                    }
                }

                setting_name = "uart_pin_rx";
                setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    char *conversion_end;
                    int32_t int_value = strtol(commands_buffer + value_position, &conversion_end, 10);
                    if (conversion_end == commands_buffer + value_end_position + 1 && int_value >= 0 && int_value < 60) {
                        ESP_LOGI(log_tag, "Valid rx pin, setting it: %ld", int_value);
                        app_settings.uart_pin_rx = (uint32_t) int_value;
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid rx pin, check limits: %ld", int_value);
                    }
                }

                setting_name = "uart_buffer_size";
                setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    char *conversion_end;
                    int32_t int_value = strtol(commands_buffer + value_position, &conversion_end, 10);
                    if (conversion_end == commands_buffer + value_end_position + 1 && int_value >= 20 && int_value < 20000) {
                        ESP_LOGI(log_tag, "Valid uart buffer size, setting it: %ld", int_value);
                        app_settings.uart_buffer_size = (uint32_t) int_value;
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid uart buffer size, check limits: %ld", int_value);
                    }
                }

                setting_name = "uart_baud_rate";
                setting_name_length = strlen(setting_name);
                if (strncmp(commands_buffer + setting_position, setting_name, setting_name_length) == 0) {
                    char *conversion_end;
                    int32_t int_value = strtol(commands_buffer + value_position, &conversion_end, 10);
                    if (conversion_end == commands_buffer + value_end_position + 1 &&
                            (
                                int_value == 110
                                || int_value == 150
                                || int_value == 300
                                || int_value == 1200
                                || int_value == 2400
                                || int_value == 4800
                                || int_value == 9600
                                || int_value == 19200
                                || int_value == 38400
                                || int_value == 57600
                                || int_value == 115200
                                || int_value == 230400
                                || int_value == 921600
                            )
                        ) {
                        ESP_LOGI(log_tag, "Valid baud rate, setting it: %ld", int_value);
                        app_settings.uart_baud_rate = (uint32_t) int_value;
                    }
                    else {
                        ESP_LOGI(log_tag, "Invalid baud rate, check possible baud rates values: %ld", int_value);
                    }
                }
            }
        }
    }

    if (commands_position >= 4 && strncmp(commands_buffer, "list", 4) == 0) {
        settings_user_list(&app_settings);
    }

    if (commands_position >= 4 && strncmp(commands_buffer, "save", 4) == 0) {
        settings_save(&app_settings);
        esp_restart();
    }

    if (commands_position >= 7 && strncmp(commands_buffer, "restore", 7) == 0) {
        settings_remove_files();
        esp_restart();
    }

    if (commands_position >= 7 && strncmp(commands_buffer, "restart", 7) == 0) {
        esp_restart();
    }

    return 1;
}

uint8_t commands_module_process_command() {
    char* newline_position = strchr(commands_buffer, '\r');
    int command_length = newline_position - commands_buffer;
    if (command_length) {
        commands_module_process_one_command(command_length);
    }
    memcpy(commands_buffer, commands_buffer + command_length + 1, COMMANDS_MODULE_BUFFER_SIZE - command_length - 1);
    commands_position -= command_length + 1;
    return 1;
}

