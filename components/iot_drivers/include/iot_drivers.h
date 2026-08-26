/**
 * @file iot_drivers.h
 * @brief Umbrella HAL/driver header. Application code includes this, not ESP-IDF driver/*.
 */
#ifndef IOT_DRIVERS_H
#define IOT_DRIVERS_H

#include "iot_gpio.h"
#include "iot_uart.h"
#include "iot_i2c.h"
#include "iot_spi.h"
#include "iot_adc.h"
#include "iot_pwm.h"
#include "iot_timer.h"
#include "iot_exti.h"
#include "iot_wdt.h"

#ifdef __cplusplus
extern "C" {
#endif

iot_err_t iot_drivers_init(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_DRIVERS_H */
