/**
 * @file iot_drivers.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Aggregated driver bring-up used by startup.
 */
#include "iot_drivers.h"
#include "iot_board.h"
#include "iot_log.h"

static const char *TAG = "iot_drv";

iot_err_t iot_drivers_init(void)
{
    iot_err_t e = iot_gpio_init();
    if (IOT_FAIL(e) && (e != IOT_ERR_ALREADY_INIT)) {
        return e;
    }
    e = iot_wdt_init();
    if (IOT_FAIL(e)) {
        return e;
    }
    iot_uart_cfg_t u0 = {
        .port = IOT_UART_PORT_0,
        .tx_pin = IOT_PIN_UART0_TX,
        .rx_pin = IOT_PIN_UART0_RX,
        .baud = 115200,
        .rs485_de_enable = false,
        .de_pin = -1,
    };
    (void)iot_uart_init(&u0);
    IOT_LOGI(TAG, "drivers ready");
    return IOT_OK;
}
