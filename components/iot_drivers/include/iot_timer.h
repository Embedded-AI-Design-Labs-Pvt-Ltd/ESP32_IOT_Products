/**
 * @file iot_timer.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief GPTimer HAL for periodic sampling (not FreeRTOS software timers).
 */
#ifndef IOT_TIMER_H
#define IOT_TIMER_H

#include <stdint.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*iot_hw_timer_cb_t)(void *arg);

iot_err_t iot_hw_timer_init(uint32_t period_us, iot_hw_timer_cb_t cb, void *arg);
iot_err_t iot_hw_timer_deinit(void);
iot_err_t iot_hw_timer_start(void);
iot_err_t iot_hw_timer_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_TIMER_H */
