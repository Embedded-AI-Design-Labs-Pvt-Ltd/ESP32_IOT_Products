/**
 * @file iot_wdt.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Task watchdog HAL wrapping esp_task_wdt.
 */
#ifndef IOT_WDT_H
#define IOT_WDT_H

#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

iot_err_t iot_wdt_init(void);
iot_err_t iot_wdt_deinit(void);
iot_err_t iot_wdt_add_current(void);
iot_err_t iot_wdt_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_WDT_H */
