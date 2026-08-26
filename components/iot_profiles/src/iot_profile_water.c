/**
 * @file iot_profile_water.c
 * @brief Smart water meter — metering/leak/valve use cases.
 */
#include "iot_profile.h"
#include "iot_exti.h"
#include "iot_adc.h"
#include "iot_gpio.h"
#include "iot_board.h"
#include "iot_services.h"
#include "iot_log.h"
#include "iot_tasks.h"
#include "iot_uc.h"

#include <stdio.h>
#include <string.h>

static const char *TAG = "water";
static uint32_t s_last_shown;

iot_err_t iot_profile_water_init(void)
{
    (void)iot_adc_init(IOT_PIN_ADC_V);
    (void)iot_gpio_config(IOT_PIN_PWM_OUT, IOT_GPIO_OUT);
    iot_task_ctx_t *ctx = iot_tasks_ctx();
    if ((ctx != NULL) && (ctx->sensor_isr_sem != NULL)) {
        (void)iot_exti_init(IOT_PIN_EXTI_PULSE, IOT_EXTI_POSEDGE, ctx->sensor_isr_sem);
    }
    IOT_LOGI(TAG, "water meter — flow sensor + valve actuator required");
    return IOT_OK;
}

iot_err_t iot_profile_water_sensor(iot_telemetry_msg_t *out)
{
    uint32_t pulses = iot_exti_count(IOT_PIN_EXTI_PULSE);
    int mv = 0;
    iot_uc_out_t uco;
    (void)iot_adc_read_mv(IOT_PIN_ADC_V, &mv);
    (void)iot_uc_water_on_sample(pulses, mv, &uco);
    s_last_shown = pulses;
    (void)s_last_shown;
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_water_cmd(const iot_command_msg_t *cmd)
{
    if (cmd == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (iot_uc_json_has_key((const char *)cmd->payload, "uc")) {
        return iot_services_handle_command(cmd);
    }
    if (strstr((const char *)cmd->payload, "close") != NULL) {
        iot_uc_out_t o;
        return iot_uc_run_json("{\"uc\":\"water.valve_close\"}", 28, &o);
    }
    if (strstr((const char *)cmd->payload, "open") != NULL) {
        iot_uc_out_t o;
        return iot_uc_run_json("{\"uc\":\"water.valve_open\"}", 27, &o);
    }
    return iot_services_handle_command(cmd);
}
