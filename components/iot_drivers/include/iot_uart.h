/**
 * @file iot_uart.h
 * @brief UART HAL. UART1 + IOT_PIN_RS485_DE is the RS485 path (external transceiver).
 */
#ifndef IOT_UART_H
#define IOT_UART_H

#include <stdint.h>
#include <stddef.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_UART_PORT_0 = 0,
    IOT_UART_PORT_1 = 1
} iot_uart_port_t;

typedef struct {
    iot_uart_port_t port;
    int tx_pin;
    int rx_pin;
    int baud;
    bool rs485_de_enable;
    int de_pin;
} iot_uart_cfg_t;

iot_err_t iot_uart_init(const iot_uart_cfg_t *cfg);
iot_err_t iot_uart_deinit(iot_uart_port_t port);
iot_err_t iot_uart_write(iot_uart_port_t port, const uint8_t *data, size_t len, uint32_t timeout_ms);
iot_err_t iot_uart_read(iot_uart_port_t port, uint8_t *data, size_t len, size_t *out_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* IOT_UART_H */
