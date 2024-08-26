/**
 * Project: ESP32 Ble Serial Bridge
 * Author: riozebratubo
 * Copyright: (c) riozebratubo
 * Project license: CC BY-NC 4.0
 **/

#include <string.h>

#include "utils.h"

uint8_t* uuid_string_to_bytearray(char* uuid) {
    uint8_t* bytes = (uint8_t*) malloc(16);
    memset(bytes, '\0', 16);
    int i = 35;
    int j = 0;
    while (i >= 1) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            i--;
            continue;
        }
        if (uuid[i] >= '0' && uuid[i] <= '9') {
            bytes[j] = uuid[i] - '0';
        }
        else if (uuid[i] >= 'A' && uuid[i] <= 'F') {
            bytes[j] = uuid[i] - 'A' + 10;
        }
        else if (uuid[i] >= 'a' && uuid[i] <= 'f') {
            bytes[j] = uuid[i] - 'a' + 10;
        }
        i--;
        if (uuid[i] >= '0' && uuid[i] <= '9') {
            bytes[j] += (uuid[i] - '0') * 16;
        }
        else if (uuid[i] >= 'A' && uuid[i] <= 'F') {
            bytes[j] += (uuid[i] - 'A' + 10) * 16;
        }
        else if (uuid[i] >= 'a' && uuid[i] <= 'f') {
            bytes[j] += (uuid[i] - 'a' + 10) * 16;
        }
        i--;
        j++;
    }
    return bytes;
}

char* uuid_bytearray_to_string(uint8_t* uuid) {
    char* str = (char*) malloc(37);
    memset(str, '\0', 37);
    sprintf(str,
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uuid[15], uuid[14], uuid[13], uuid[12], uuid[11], uuid[10], uuid[9], uuid[8],
        uuid[7], uuid[6], uuid[5], uuid[4], uuid[3], uuid[2], uuid[1], uuid[0]
    );
    return str;
}

uint8_t uuid_string_is_valid(char* uuid) {
    int length = strlen(uuid);
    if (length != 36) return 0;
    if (uuid[8] != '-' || uuid[13] != '-' || uuid[18] != '-' || uuid[23] != '-') return 0;
    for (int i = 0; i < 36; i++) {
       if (i == 8 || i == 13 || i == 18 || i == 23) continue;
       if (!(uuid[i] >= '0' && uuid[i] <= '9') && !(uuid[i] >= 'A' && uuid[i] <= 'F') && !(uuid[i] >= 'a' && uuid[i] <= 'f')) return 0;
    }
    return 1;
}

uint8_t uuid_char_array_is_valid(char* uuid) {
    if (uuid[8] != '-' || uuid[13] != '-' || uuid[18] != '-' || uuid[23] != '-') return 0;
    for (int i = 0; i < 36; i++) {
       if (i == 8 || i == 13 || i == 18 || i == 23) continue;
       if (!(uuid[i] >= '0' && uuid[i] <= '9') && !(uuid[i] >= 'A' && uuid[i] <= 'F') && !(uuid[i] >= 'a' && uuid[i] <= 'f')) return 0;
    }
    return 1;
}

uint8_t* mac_address_string_to_bytearray(char* mac) {
    uint8_t* bytes = (uint8_t*) malloc(6);
    memset(bytes, '\0', 6);
    
    int i = 16;
    int j = 5;
    while (i >= 1) {
        if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14) {
            i--;
            continue;
        }
        if (mac[i] >= '0' && mac[i] <= '9') {
            bytes[j] = mac[i] - '0';
        }
        else if (mac[i] >= 'A' && mac[i] <= 'F') {
            bytes[j] = mac[i] - 'A' + 10;
        }
        else if (mac[i] >= 'a' && mac[i] <= 'f') {
            bytes[j] = mac[i] - 'a' + 10;
        }
        i--;
        if (mac[i] >= '0' && mac[i] <= '9') {
            bytes[j] += (mac[i] - '0') * 16;
        }
        else if (mac[i] >= 'A' && mac[i] <= 'F') {
            bytes[j] += (mac[i] - 'A' + 10) * 16;
        }
        else if (mac[i] >= 'a' && mac[i] <= 'f') {
            bytes[j] += (mac[i] - 'a' + 10) * 16;
        }
        i--;
        j--;
    }
    return bytes;
}

uint8_t mac_address_char_array_is_valid(char* mac) {
    if (mac[2] != ':' || mac[5] != ':' || mac[8] != ':' || mac[11] != ':' || mac[14] != ':') return 0;
    for (int i = 0; i < 17; i++) {
       if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14) continue;
       if (!(mac[i] >= '0' && mac[i] <= '9') && !(mac[i] >= 'A' && mac[i] <= 'F') && !(mac[i] >= 'a' && mac[i] <= 'f')) return 0;
    }
    return 1;
}
