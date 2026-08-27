/**
 * @file iot_profile.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Selects compiled-in product profile (Kconfig).
 */
#include "iot_profile.h"
#include "iot_log.h"
#include "iot_services.h"

static const char *TAG = "iot_profile";
static iot_sensor_hook_t s_sensor;
static iot_proc_hook_t s_proc;
static iot_cmd_hook_t s_cmd;
static const char *s_name = "none";

static iot_err_t passthrough_proc(const iot_telemetry_msg_t *in, iot_telemetry_msg_t *out)
{
    if ((in == NULL) || (out == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    *out = *in;
    return IOT_OK;
}

const char *iot_profile_name(void)
{
    return s_name;
}

iot_sensor_hook_t iot_profile_sensor_hook(void)
{
    return s_sensor;
}

iot_proc_hook_t iot_profile_proc_hook(void)
{
    return s_proc != NULL ? s_proc : passthrough_proc;
}

iot_cmd_hook_t iot_profile_cmd_hook(void)
{
    return s_cmd;
}

iot_err_t iot_profile_dispatch_cmd(const iot_command_msg_t *cmd)
{
    return iot_services_handle_command(cmd);
}

iot_err_t iot_profile_init(void)
{
#if defined(CONFIG_IOT_PROFILE_INDUSTRIAL)
    s_name = "industrial";
    s_sensor = iot_profile_industrial_sensor;
    s_cmd = iot_profile_industrial_cmd;
    return iot_profile_industrial_init();
#elif defined(CONFIG_IOT_PROFILE_PREDICTIVE)
    s_name = "predictive";
    s_sensor = iot_profile_predictive_sensor;
    s_cmd = iot_profile_predictive_cmd;
    return iot_profile_predictive_init();
#elif defined(CONFIG_IOT_PROFILE_WATER)
    s_name = "water";
    s_sensor = iot_profile_water_sensor;
    s_cmd = iot_profile_water_cmd;
    return iot_profile_water_init();
#elif defined(CONFIG_IOT_PROFILE_EV_CHARGER)
    s_name = "ev_charger";
    s_sensor = iot_profile_ev_sensor;
    s_cmd = iot_profile_ev_cmd;
    return iot_profile_ev_init();
#elif defined(CONFIG_IOT_PROFILE_AUTOMOTIVE)
    s_name = "automotive";
    s_sensor = iot_profile_automotive_sensor;
    s_cmd = iot_profile_automotive_cmd;
    return iot_profile_automotive_init();
#elif defined(CONFIG_IOT_PROFILE_ASSET_TRACKER)
    s_name = "asset_tracker";
    s_sensor = iot_profile_tracker_sensor;
    s_cmd = iot_profile_tracker_cmd;
    return iot_profile_tracker_init();
#elif defined(CONFIG_IOT_PROFILE_AGRICULTURE)
    s_name = "agriculture";
    s_sensor = iot_profile_agri_sensor;
    s_cmd = iot_profile_agri_cmd;
    return iot_profile_agri_init();
#elif defined(CONFIG_IOT_PROFILE_COLDCHAIN)
    s_name = "coldchain";
    s_sensor = iot_profile_coldchain_sensor;
    s_cmd = iot_profile_coldchain_cmd;
    return iot_profile_coldchain_init();
#elif defined(CONFIG_IOT_PROFILE_ROBOTICS)
    s_name = "robotics";
    s_sensor = iot_profile_robotics_sensor;
    s_cmd = iot_profile_robotics_cmd;
    return iot_profile_robotics_init();
#else
    s_name = "energy";
    s_sensor = iot_profile_energy_sensor;
    s_cmd = iot_profile_energy_cmd;
    IOT_LOGI(TAG, "profile=%s", s_name);
    return iot_profile_energy_init();
#endif
}
