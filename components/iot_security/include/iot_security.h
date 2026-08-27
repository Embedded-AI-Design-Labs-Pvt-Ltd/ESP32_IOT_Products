/**
 * @file iot_security.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Device identity, TLS policy, and NVS protection strategy (no hardcoded secrets).
 */
#ifndef IOT_SECURITY_H
#define IOT_SECURITY_H

#include "iot_err.h"
#include "iot_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

iot_err_t iot_security_init(void);
iot_err_t iot_security_load_identity(iot_identity_t *out);
iot_err_t iot_security_validate_command(const iot_command_msg_t *cmd);
/** Derive a stable device id from MAC when unprovisioned. */
iot_err_t iot_security_default_device_id(char *out, size_t out_len);

#ifdef __cplusplus
}
#endif

#endif /* IOT_SECURITY_H */
