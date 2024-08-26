/**
 * Project: ESP32 Ble Serial Bridge
 * Author: riozebratubo
 * Copyright: (c) riozebratubo
 * Project license: CC BY-NC 4.0
 **/

#include "settings.h"

#include <string.h>

#include "esp_log.h"
#include "esp_spiffs.h"

#include "utils.h"

static char* log_tag = "SERIALBLE_SETTINGS";

esp_vfs_spiffs_conf_t settings_spiffs_conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
};

uint8_t settings_data_valid(app_settings_t* app_settings) {
    if (!uuid_char_array_is_valid(app_settings->complete_service_uuid)) {
        ESP_LOGE(log_tag, "Error: cannot start service. Bad service uuid: %.36s", app_settings->complete_service_uuid);
        return 0;
    }

    if (!uuid_char_array_is_valid(app_settings->complete_characteristic_uuid)) {
        ESP_LOGE(log_tag, "Error: cannot start service. Bad characteristic uuid: %.36s", app_settings->complete_characteristic_uuid);
        return 0;
    }

    return 1;
}

uint8_t settings_init() {
    esp_err_t ret1 = esp_vfs_spiffs_register(&settings_spiffs_conf);
    if (ret1 != ESP_OK) {
        if (ret1 == ESP_FAIL) {
            ESP_LOGE(log_tag, "Failed to mount or format filesystem");
        } else if (ret1 == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(log_tag, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(log_tag, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret1));
        }
        return 0;
    }
    
    ESP_LOGI(log_tag, "Performing SPIFFS_check()...");
    ret1 = esp_spiffs_check(settings_spiffs_conf.partition_label);
    if (ret1 != ESP_OK) {
        ESP_LOGE(log_tag, "SPIFFS_check() failed (%s)", esp_err_to_name(ret1));
        return 0;
    } else {
        ESP_LOGI(log_tag, "SPIFFS_check() successful");
    }

    return 1;
}

void settings_log(app_settings_t* app_settings) {
    ESP_LOGI(log_tag, "Current settings:");
    ESP_LOGI(log_tag, "- Complete service uuid: %.*s", sizeof(app_settings->complete_service_uuid), app_settings->complete_service_uuid);
    ESP_LOGI(log_tag, "- Complete characteristic uuid: %.*s", sizeof(app_settings->complete_characteristic_uuid), app_settings->complete_characteristic_uuid);
    ESP_LOGI(log_tag, "- Device name: %.*s", sizeof(app_settings->device_name), app_settings->device_name);
    ESP_LOGI(log_tag, "- Manufacturer bytes: %02x-%02x-%02x-%02x", app_settings->manufacturer_bytes[0], app_settings->manufacturer_bytes[1], app_settings->manufacturer_bytes[2], app_settings->manufacturer_bytes[3]);
    ESP_LOGI(log_tag, "- Desired ble mtu: %lu", app_settings->desired_ble_mtu);
    ESP_LOGI(log_tag, "- Force mac address: %.*s", sizeof(app_settings->force_mac_address), app_settings->force_mac_address);
    ESP_LOGI(log_tag, "- Uart pin tx: %lu", app_settings->uart_pin_tx);
    ESP_LOGI(log_tag, "- Uart pin rx: %lu", app_settings->uart_pin_rx);
    ESP_LOGI(log_tag, "- Uart buffer size: %lu", app_settings->uart_buffer_size);
    ESP_LOGI(log_tag, "- Uart baud rate: %lu", app_settings->uart_baud_rate);
}

void settings_user_list(app_settings_t* app_settings) {
    ESP_LOGI(log_tag, "Current settings:");
    ESP_LOGI(log_tag, "- complete_service_uuid: %.*s", sizeof(app_settings->complete_service_uuid), app_settings->complete_service_uuid);
    ESP_LOGI(log_tag, "- complete_characteristic_uuid: %.*s", sizeof(app_settings->complete_characteristic_uuid), app_settings->complete_characteristic_uuid);
    ESP_LOGI(log_tag, "- device_name: %.*s", sizeof(app_settings->device_name), app_settings->device_name);
    ESP_LOGI(log_tag, "- desired_ble_mtu: %lu", app_settings->desired_ble_mtu);
    ESP_LOGI(log_tag, "- force_mac_address: %.*s", sizeof(app_settings->force_mac_address), app_settings->force_mac_address);
    ESP_LOGI(log_tag, "- uart_pin_tx: %lu", app_settings->uart_pin_tx);
    ESP_LOGI(log_tag, "- uart_pin_rx: %lu", app_settings->uart_pin_rx);
    ESP_LOGI(log_tag, "- uart_buffer_size: %lu", app_settings->uart_buffer_size);
    ESP_LOGI(log_tag, "- uart_baud_rate: %lu", app_settings->uart_baud_rate);
}

uint8_t settings_save(app_settings_t* app_settings) {
    app_settings_version_t temp_version = {0};

    temp_version.version = CURRENT_SETTINGS_VERSION;

    {
        ESP_LOGI(log_tag, "Opening file settings version for writing...");
        FILE* f = fopen("/spiffs/app_config_version", "w");
        if (f == NULL) {
            ESP_LOGE(log_tag, "Failed to open file settings version for writing!");
            return 0;
        }
        fwrite(&temp_version, sizeof(app_settings_version_t), 1, f);
        fclose(f);
        ESP_LOGI(log_tag, "File settings version written successfully");
    }

    {
        ESP_LOGI(log_tag, "Opening file settings data for writing...");
        FILE* f = fopen("/spiffs/app_config", "w");
        if (f == NULL) {
            ESP_LOGE(log_tag, "Failed to open file settings data for writing!");
            return 0;
        }
        fwrite(app_settings, sizeof(app_settings_t), 1, f);
        fclose(f);
        ESP_LOGI(log_tag, "File settings data written successfully");
    }

    return 1;
}

uint8_t settings_load(app_settings_t* app_settings) {
    app_settings_version_t temp_version = {0};
    app_settings_t temp_settings = {0};

    temp_version.version = -1;

    ESP_LOGI(log_tag, "Trying to read app settings version...");
    {
        FILE* f = fopen("/spiffs/app_config_version", "r");
        if (f == NULL) {
            ESP_LOGE(log_tag, "Failed to open settings version file for reading!");
            return 0;
        }
        size_t obj_read_count = fread(&temp_version, sizeof(app_settings_version_t), 1, f);
        fclose(f);
        if (obj_read_count < 1) {
            ESP_LOGE(log_tag, "Could not read full settings version file!");
            return 0;
        }
        if (temp_version.version == -1) {
            ESP_LOGE(log_tag, "Read settings version, but invalid version data.");
            return 0;
        }
    }

    ESP_LOGI(log_tag, "Trying to read app settings...");
    {
        FILE* f = fopen("/spiffs/app_config", "r");
        if (f == NULL) {
            ESP_LOGE(log_tag, "Failed to open settings data file for reading!");
            return 0;
        }
        size_t obj_read_count = fread(&temp_settings, sizeof(app_settings_t), 1, f);
        fclose(f);
        if (obj_read_count < 1) {
            ESP_LOGE(log_tag, "Could not read settings data file!");
            return 0;
        }
    }

    ESP_LOGI(log_tag, "Read all sucessfully");

    if (temp_version.version < CURRENT_SETTINGS_VERSION) {
        // currently no action
    }

    if (settings_data_valid(&temp_settings)) {
        memcpy(app_settings, &temp_settings, sizeof(app_settings_t));
        return 1;
    }
    else {
        return 0;
    }
}

uint8_t settings_remove_files() {
    remove("/spiffs/app_config_version");
    remove("/spiffs/app_config");
    return 1;
}
