/**
 * @file iot_adc.c
 * @brief ADC1 oneshot HAL (esp_adc/adc_oneshot.h).
 */
#include "iot_adc.h"
#include "iot_log.h"
#include "iot_rtos.h"

#ifdef ESP_PLATFORM
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
static adc_oneshot_unit_handle_t s_adc;
static adc_cali_handle_t s_cali;
#endif

static const char *TAG = "iot_adc";
static iot_mutex_t s_lock;
static bool s_init;
static int s_sim_raw = 2048;

iot_err_t iot_adc_init(int gpio_pin)
{
    if (s_init) {
        return IOT_ERR_ALREADY_INIT;
    }
    iot_err_t e = iot_mutex_create(&s_lock);
    if (IOT_FAIL(e)) {
        return e;
    }
#ifdef ESP_PLATFORM
    adc_oneshot_unit_init_cfg_t ucfg = {.unit_id = ADC_UNIT_1};
    esp_err_t err = adc_oneshot_new_unit(&ucfg, &s_adc);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    adc_oneshot_chan_cfg_t ccfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_channel_t ch;
    adc_unit_t unit;
    err = adc_oneshot_io_to_channel(gpio_pin, &unit, &ch);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    err = adc_oneshot_config_channel(s_adc, ch, &ccfg);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cal = {
        .unit_id = ADC_UNIT_1,
        .chan = ch,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    (void)adc_cali_create_scheme_curve_fitting(&cal, &s_cali);
#endif
#else
    (void)gpio_pin;
#endif
    s_init = true;
    IOT_LOGI(TAG, "adc1 init gpio=%d", gpio_pin);
    return IOT_OK;
}

iot_err_t iot_adc_deinit(void)
{
    if (!s_init) {
        return IOT_ERR_NOT_INIT;
    }
#ifdef ESP_PLATFORM
    if (s_cali != NULL) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        (void)adc_cali_delete_scheme_curve_fitting(s_cali);
#endif
        s_cali = NULL;
    }
    if (s_adc != NULL) {
        (void)adc_oneshot_del_unit(s_adc);
        s_adc = NULL;
    }
#endif
    s_init = false;
    return IOT_OK;
}

iot_err_t iot_adc_read_raw(int gpio_pin, int *raw)
{
    if (!s_init || (raw == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock, 100U);
#ifdef ESP_PLATFORM
    adc_channel_t ch;
    adc_unit_t unit;
    esp_err_t err = adc_oneshot_io_to_channel(gpio_pin, &unit, &ch);
    if (err == ESP_OK) {
        err = adc_oneshot_read(s_adc, ch, raw);
    }
    iot_mutex_unlock(s_lock);
    return iot_err_from_esp(err);
#else
    (void)gpio_pin;
    *raw = s_sim_raw;
    iot_mutex_unlock(s_lock);
    return IOT_OK;
#endif
}

iot_err_t iot_adc_read_mv(int gpio_pin, int *mv)
{
    int raw = 0;
    iot_err_t e = iot_adc_read_raw(gpio_pin, &raw);
    if (IOT_FAIL(e)) {
        return e;
    }
#ifdef ESP_PLATFORM
    if (s_cali != NULL) {
        return iot_err_from_esp(adc_cali_raw_to_voltage(s_cali, raw, mv));
    }
#endif
    if (mv == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    /* Uncalibrated fallback: 12-bit full scale ~ 3300 mV at 12 dB atten (approx). */
    *mv = (raw * 3300) / 4095;
    return IOT_OK;
}

void iot_adc_sim_set_raw(int raw)
{
    s_sim_raw = raw;
}
