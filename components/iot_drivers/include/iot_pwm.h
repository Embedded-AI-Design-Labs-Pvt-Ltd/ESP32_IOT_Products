/**
 * @file iot_pwm.h
 * @brief LEDC PWM HAL.
 */
#ifndef IOT_PWM_H
#define IOT_PWM_H

#include <stdint.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

iot_err_t iot_pwm_init(int gpio_pin, uint32_t freq_hz, uint32_t resolution_bits);
iot_err_t iot_pwm_deinit(void);
iot_err_t iot_pwm_set_duty(uint32_t duty);

#ifdef __cplusplus
}
#endif

#endif /* IOT_PWM_H */
