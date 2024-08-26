/**
 * Project: ESP32 Ble Serial Bridge
 * Author: riozebratubo
 * Copyright: (c) riozebratubo
 * Project license: CC BY-NC 4.0
 **/

#include <stdio.h>

uint8_t* uuid_string_to_bytearray(char* uuid);
char* uuid_bytearray_to_string(uint8_t* uuid);
uint8_t uuid_string_is_valid(char* uuid);
uint8_t uuid_char_array_is_valid(char* uuid);
uint8_t* mac_address_string_to_bytearray(char* mac);
uint8_t mac_address_char_array_is_valid(char* mac);
