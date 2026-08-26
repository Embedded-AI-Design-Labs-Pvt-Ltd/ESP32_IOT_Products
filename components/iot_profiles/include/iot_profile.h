/**
 * @file iot_profile.h
 * @brief Product profile interface. Profiles use HAL + IoT services only.
 */
#ifndef IOT_PROFILE_H
#define IOT_PROFILE_H

#include "iot_err.h"
#include "iot_hooks.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *iot_profile_name(void);
iot_err_t iot_profile_init(void);
iot_sensor_hook_t iot_profile_sensor_hook(void);
iot_proc_hook_t iot_profile_proc_hook(void);
iot_cmd_hook_t iot_profile_cmd_hook(void);

iot_err_t iot_profile_energy_init(void);
iot_err_t iot_profile_industrial_init(void);
iot_err_t iot_profile_predictive_init(void);
iot_err_t iot_profile_water_init(void);
iot_err_t iot_profile_ev_init(void);
iot_err_t iot_profile_automotive_init(void);
iot_err_t iot_profile_tracker_init(void);
iot_err_t iot_profile_agri_init(void);
iot_err_t iot_profile_coldchain_init(void);
iot_err_t iot_profile_robotics_init(void);

iot_err_t iot_profile_energy_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_industrial_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_predictive_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_water_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_ev_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_automotive_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_tracker_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_agri_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_coldchain_sensor(iot_telemetry_msg_t *out);
iot_err_t iot_profile_robotics_sensor(iot_telemetry_msg_t *out);

iot_err_t iot_profile_energy_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_industrial_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_predictive_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_water_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_ev_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_automotive_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_tracker_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_agri_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_coldchain_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_robotics_cmd(const iot_command_msg_t *cmd);
iot_err_t iot_profile_dispatch_cmd(const iot_command_msg_t *cmd);

#ifdef __cplusplus
}
#endif

#endif /* IOT_PROFILE_H */
