/**
 * @file iot_hooks.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Optional profile hooks installed before iot_tasks_start().
 */
#ifndef IOT_HOOKS_H
#define IOT_HOOKS_H

#include "iot_err.h"
#include "iot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef iot_err_t (*iot_sensor_hook_t)(iot_telemetry_msg_t *out);
typedef iot_err_t (*iot_proc_hook_t)(const iot_telemetry_msg_t *in, iot_telemetry_msg_t *out);
typedef iot_err_t (*iot_cmd_hook_t)(const iot_command_msg_t *cmd);

void iot_tasks_set_hooks(iot_sensor_hook_t sensor, iot_proc_hook_t proc, iot_cmd_hook_t cmd);

#ifdef __cplusplus
}
#endif

#endif /* IOT_HOOKS_H */
