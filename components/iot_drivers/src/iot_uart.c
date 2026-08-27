/**
 * @file iot_uart.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief UART HAL with optional RS485 DE/RE pin (external transceiver required).
 */
#include "iot_uart.h"
#include "iot_gpio.h"
#include "iot_log.h"
#include "iot_rtos.h"

#ifdef ESP_PLATFORM
#include "driver/uart.h"
#endif

static const char *TAG = "iot_uart";
static iot_mutex_t s_lock[2];
static iot_uart_cfg_t s_cfg[2];
static bool s_ready[2];

iot_err_t iot_uart_init(const iot_uart_cfg_t *cfg)
{
    if ((cfg == NULL) || ((int)cfg->port > 1)) {
        return IOT_ERR_INVALID_ARG;
    }
    const int p = (int)cfg->port;
    if (!s_ready[p]) {
        iot_err_t e = iot_mutex_create(&s_lock[p]);
        if (IOT_FAIL(e)) {
            return e;
        }
    }
    s_cfg[p] = *cfg;
#ifdef ESP_PLATFORM
    uart_config_t uc = {
        .baud_rate = cfg->baud > 0 ? cfg->baud : 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_port_t port = (p == 0) ? UART_NUM_0 : UART_NUM_1;
    esp_err_t err = uart_driver_install(port, 1024, 1024, 0, NULL, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return iot_err_from_esp(err);
    }
    err = uart_param_config(port, &uc);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    err = uart_set_pin(port, cfg->tx_pin, cfg->rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
#endif
    if (cfg->rs485_de_enable) {
        (void)iot_gpio_config(cfg->de_pin, IOT_GPIO_OUT);
        (void)iot_gpio_write(cfg->de_pin, false);
        IOT_LOGI(TAG, "RS485 DE/RE on GPIO %d (external transceiver required)", cfg->de_pin);
    }
    s_ready[p] = true;
    IOT_LOGI(TAG, "uart%d baud=%d", p, cfg->baud);
    return IOT_OK;
}

iot_err_t iot_uart_deinit(iot_uart_port_t port)
{
    const int p = (int)port;
    if ((p > 1) || !s_ready[p]) {
        return IOT_ERR_NOT_INIT;
    }
#ifdef ESP_PLATFORM
    (void)uart_driver_delete((p == 0) ? UART_NUM_0 : UART_NUM_1);
#endif
    s_ready[p] = false;
    return IOT_OK;
}

iot_err_t iot_uart_write(iot_uart_port_t port, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    const int p = (int)port;
    if ((p > 1) || !s_ready[p] || (data == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock[p], timeout_ms);
    if (s_cfg[p].rs485_de_enable) {
        (void)iot_gpio_write(s_cfg[p].de_pin, true);
    }
#ifdef ESP_PLATFORM
    int n = uart_write_bytes((p == 0) ? UART_NUM_0 : UART_NUM_1, data, len);
    (void)uart_wait_tx_done((p == 0) ? UART_NUM_0 : UART_NUM_1, pdMS_TO_TICKS(timeout_ms));
#else
    int n = (int)len;
    (void)timeout_ms;
#endif
    if (s_cfg[p].rs485_de_enable) {
        (void)iot_gpio_write(s_cfg[p].de_pin, false);
    }
    iot_mutex_unlock(s_lock[p]);
    return (n < 0) ? IOT_ERR_IO : IOT_OK;
}

iot_err_t iot_uart_read(iot_uart_port_t port, uint8_t *data, size_t len, size_t *out_len, uint32_t timeout_ms)
{
    const int p = (int)port;
    if ((p > 1) || !s_ready[p] || (data == NULL) || (out_len == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
#ifdef ESP_PLATFORM
    int n = uart_read_bytes((p == 0) ? UART_NUM_0 : UART_NUM_1, data, len, pdMS_TO_TICKS(timeout_ms));
    *out_len = (n > 0) ? (size_t)n : 0U;
    return (n < 0) ? IOT_ERR_IO : IOT_OK;
#else
    (void)len;
    (void)timeout_ms;
    *out_len = 0U;
    return IOT_ERR_TIMEOUT;
#endif
}
