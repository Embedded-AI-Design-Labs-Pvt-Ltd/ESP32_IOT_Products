/**
 * @file iot_provision.h
 * @brief BLE/Wi-Fi device provisioning (no production secrets in firmware).
 */
#ifndef IOT_PROVISION_H
#define IOT_PROVISION_H

#include "iot_err.h"
#include "iot_config.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

iot_err_t iot_provision_init(void);
iot_err_t iot_provision_apply_json(const char *json, size_t len);
bool iot_provision_is_complete(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_PROVISION_H */
