/**
 * @file iot_profile_predictive.c
 * @brief Predictive maintenance — RMS/peak/crest/health use cases.
 */
#include "iot_profile.h"
#include "iot_adc.h"
#include "iot_board.h"
#include "iot_services.h"
#include "iot_log.h"
#include "iot_uc.h"

iot_err_t iot_profile_predictive_init(void)
{
    (void)iot_adc_init(IOT_PIN_ADC_V);
    IOT_LOGI("pred", "accelerometer/current features — analog or I2C IMU required");
    return IOT_OK;
}

iot_err_t iot_profile_predictive_sensor(iot_telemetry_msg_t *out)
{
    int mv = 0;
    iot_uc_out_t uco;
    (void)iot_adc_read_mv(IOT_PIN_ADC_V, &mv);
    float x = ((float)mv - 1650.0f) / 100.0f;
    (void)iot_uc_pm_on_sample(x, &uco);
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_predictive_cmd(const iot_command_msg_t *cmd)
{
    return iot_profile_dispatch_cmd(cmd);
}
