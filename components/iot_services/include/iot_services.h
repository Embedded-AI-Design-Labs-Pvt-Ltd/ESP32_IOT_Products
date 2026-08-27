/**
 * @file iot_services.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief IoT service facade: telemetry JSON helpers and command dispatch.
 */
#ifndef IOT_SERVICES_H
#define IOT_SERVICES_H

#include "iot_err.h"
#include "iot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

iot_err_t iot_services_init(void);
iot_err_t iot_services_handle_command(const iot_command_msg_t *cmd);
iot_err_t iot_services_fill_telemetry(iot_telemetry_msg_t *msg, const char *json);

#ifdef __cplusplus
}
#endif

#endif /* IOT_SERVICES_H */
