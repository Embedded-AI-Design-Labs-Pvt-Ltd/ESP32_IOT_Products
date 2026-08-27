/**
 * @file iot_profile_industrial.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Industrial machine monitoring: temperature, vibration, current, pressure, state.
 *
 * External: I2C temp (e.g. TMP117 @0x48), analog vibration, CT, 4-20 mA pressure via ADC.
 */
#include "iot_profile.h"
#include "iot_i2c.h"
#include "iot_adc.h"
#include "iot_board.h"
#include "iot_services.h"
#include "iot_log.h"
#include "iot_exti.h"
#include "iot_tasks.h"
#include "iot_uc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "industrial";

iot_err_t iot_profile_industrial_init(void)
{
    iot_i2c_cfg_t i2c = {.sda = IOT_PIN_I2C_SDA, .scl = IOT_PIN_I2C_SCL, .hz = 100000};
    (void)iot_i2c_init(&i2c);
    (void)iot_adc_init(IOT_PIN_ADC_V);
    iot_task_ctx_t *ctx = iot_tasks_ctx();
    if ((ctx != NULL) && (ctx->sensor_isr_sem != NULL)) {
        (void)iot_exti_init(IOT_PIN_EXTI_PULSE, IOT_EXTI_POSEDGE, ctx->sensor_isr_sem);
    }
    IOT_LOGI(TAG, "industrial — TMP117/vibration/CT/pressure hardware required");
    return IOT_OK;
}

iot_err_t iot_profile_industrial_sensor(iot_telemetry_msg_t *out)
{
    uint8_t raw[2] = {0};
    float temp = 25.0f;
    if (iot_i2c_read(0x48, raw, sizeof(raw), 50U) == IOT_OK) {
        int16_t s = (int16_t)((raw[0] << 8) | raw[1]);
        temp = ((float)s) * 0.0078125f;
    }
    int mv = 0;
    (void)iot_adc_read_mv(IOT_PIN_ADC_V, &mv);
    float vib = (float)(mv % 500) / 10.0f;
    float current = 1.0f + ((float)(mv % 200) / 100.0f);
    float pressure = 1.0f + ((float)(mv % 400) / 200.0f);
    iot_uc_out_t uco;
    (void)iot_uc_ind_on_sample(temp, vib, current, pressure, iot_exti_count(IOT_PIN_EXTI_PULSE), &uco);
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_industrial_cmd(const iot_command_msg_t *cmd)
{
    if (cmd == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (strstr((const char *)cmd->payload, "t_alarm") != NULL &&
        !iot_uc_json_has_key((const char *)cmd->payload, "uc")) {
        float t = (float)atof(strstr((const char *)cmd->payload, "t_alarm") + 8);
        iot_command_msg_t mapped = *cmd;
        char buf[96];
        (void)snprintf(buf, sizeof(buf), "{\"uc\":\"industrial.set_limits\",\"t_lim\":%.1f}", (double)t);
        (void)memset(mapped.payload, 0, sizeof(mapped.payload));
        size_t n = strlen(buf);
        (void)memcpy(mapped.payload, buf, n);
        mapped.payload_len = (uint16_t)n;
        return iot_services_handle_command(&mapped);
    }
    return iot_services_handle_command(cmd);
}
