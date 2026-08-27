/**
 * @file iot_profile_ev.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief EV charger IoT gateway — non-safety use cases only.
 */
#include "iot_profile.h"
#include "iot_adc.h"
#include "iot_board.h"
#include "iot_services.h"
#include "iot_log.h"
#include "iot_uc.h"

#include <string.h>

static const char *TAG = "evgw";

iot_err_t iot_profile_ev_init(void)
{
    (void)iot_adc_init(IOT_PIN_ADC_V);
    IOT_LOGW(TAG, "NOT a safety charger controller — connectivity gateway only");
    return IOT_OK;
}

iot_err_t iot_profile_ev_sensor(iot_telemetry_msg_t *out)
{
    int mv = 0;
    iot_uc_out_t uco;
    (void)iot_adc_read_mv(IOT_PIN_ADC_V, &mv);
    (void)iot_uc_ev_on_sample(mv, &uco);
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_ev_cmd(const iot_command_msg_t *cmd)
{
    if (cmd == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (strstr((const char *)cmd->payload, "contactor") != NULL) {
        iot_uc_out_t o;
        return iot_uc_run_json("{\"uc\":\"ev.reject_contactor\"}", 32, &o);
    }
    return iot_services_handle_command(cmd);
}
