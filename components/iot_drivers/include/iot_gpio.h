/**
 * @file iot_gpio.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Thread-safe GPIO HAL wrapping ESP-IDF driver/gpio.h.
 */
#ifndef IOT_GPIO_H
#define IOT_GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_GPIO_IN = 0,
    IOT_GPIO_OUT,
    IOT_GPIO_IN_PULLUP,
    IOT_GPIO_IN_PULLDOWN
} iot_gpio_mode_t;

iot_err_t iot_gpio_init(void);
iot_err_t iot_gpio_deinit(void);
iot_err_t iot_gpio_config(int pin, iot_gpio_mode_t mode);
iot_err_t iot_gpio_write(int pin, bool level);
iot_err_t iot_gpio_read(int pin, bool *level);
iot_err_t iot_gpio_toggle(int pin);

#ifdef __cplusplus
}
#endif

#endif /* IOT_GPIO_H */
