/**
 * @file iot_ota.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief HTTPS OTA with rollback and watchdog feeding during download.
 */
#ifndef IOT_OTA_H
#define IOT_OTA_H

#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_OTA_IDLE = 0,
    IOT_OTA_DOWNLOADING,
    IOT_OTA_VERIFY,
    IOT_OTA_REBOOT,
    IOT_OTA_FAIL,
    IOT_OTA_ROLLBACK
} iot_ota_state_t;

iot_err_t iot_ota_init(void);
iot_err_t iot_ota_start(const char *url);
iot_ota_state_t iot_ota_state(void);
iot_err_t iot_ota_mark_valid(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_OTA_H */
