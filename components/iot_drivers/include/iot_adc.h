/**
 * @file iot_adc.h
 * @brief ADC1 oneshot HAL (ESP32-C3 has ADC1 only for oneshot in typical IDF configs).
 */
#ifndef IOT_ADC_H
#define IOT_ADC_H

#include <stdint.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

iot_err_t iot_adc_init(int gpio_pin);
iot_err_t iot_adc_deinit(void);
iot_err_t iot_adc_read_raw(int gpio_pin, int *raw);
iot_err_t iot_adc_read_mv(int gpio_pin, int *mv);

#ifdef __cplusplus
}
#endif

#endif /* IOT_ADC_H */
