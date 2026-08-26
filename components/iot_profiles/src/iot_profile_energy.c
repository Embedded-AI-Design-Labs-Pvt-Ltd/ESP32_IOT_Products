/**
 * @file iot_profile_energy.c
 * @brief Smart Energy Monitor — delegates detailed use cases to iot_uc_energy_*.
 */
#include "iot_profile.h"
#include "iot_adc.h"
#include "iot_board.h"
#include "iot_services.h"
#include "iot_log.h"
#include "iot_gpio.h"
#include "iot_uc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *TAG = "energy";

iot_err_t iot_profile_energy_init(void)
{
    (void)iot_adc_init(IOT_PIN_ADC_V);
    (void)iot_gpio_config(IOT_PIN_LED, IOT_GPIO_OUT);
    (void)iot_uc_init();
    IOT_LOGI(TAG, "energy monitor — CT/shunt + voltage divider required");
    return IOT_OK;
}

iot_err_t iot_profile_energy_sensor(iot_telemetry_msg_t *out)
{
    int mv = 0;
    iot_uc_out_t uco;
    (void)iot_adc_read_mv(IOT_PIN_ADC_V, &mv);
    iot_err_t e = iot_uc_energy_on_adc(mv, &uco);
    if (IOT_FAIL(e)) {
        return e;
    }
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_energy_cmd(const iot_command_msg_t *cmd)
{
    if ((cmd == NULL) || (cmd->payload_len == 0U)) {
        return IOT_ERR_INVALID_ARG;
    }
    if (iot_uc_json_has_key((const char *)cmd->payload, "uc")) {
        return iot_services_handle_command(cmd);
    }
    if (strstr((const char *)cmd->payload, "alarm_w") != NULL) {
        iot_command_msg_t mapped = *cmd;
        /* Legacy alarm_w → energy.set_limits */
        char buf[128];
        float w = (float)atof(strstr((const char *)cmd->payload, "alarm_w") + 8);
        (void)snprintf(buf, sizeof(buf), "{\"uc\":\"energy.set_limits\",\"over_w\":%.1f}", (double)w);
        (void)memset(mapped.payload, 0, sizeof(mapped.payload));
        size_t n = strlen(buf);
        (void)memcpy(mapped.payload, buf, n);
        mapped.payload_len = (uint16_t)n;
        return iot_services_handle_command(&mapped);
    }
    return iot_services_handle_command(cmd);
}
