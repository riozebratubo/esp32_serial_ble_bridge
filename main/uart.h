/**
 * Project: ESP32 Ble Serial Bridge
 * Author: riozebratubo
 * Copyright: (c) riozebratubo
 * Project license: CC BY-NC 4.0
 **/

#include "driver/uart.h"

void uart_module_init();
void uart_module_start_task();
int uart_module_write_bytes(uart_port_t uart_num, const void *src, size_t size);
