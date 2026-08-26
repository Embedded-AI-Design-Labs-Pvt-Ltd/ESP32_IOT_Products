/**
 * @file iot_pwm.c
 * @brief LEDC PWM HAL.
 */
#include "iot_pwm.h"
#include "iot_log.h"

#ifdef ESP_PLATFORM
#include "driver/ledc.h"
#endif

static const char *TAG = "iot_pwm";
static bool s_init;
static uint32_t s_duty_max = 1023U;

iot_err_t iot_pwm_init(int gpio_pin, uint32_t freq_hz, uint32_t resolution_bits)
{
    if (s_init) {
        return IOT_ERR_ALREADY_INIT;
    }
    if (resolution_bits < 8U) {
        resolution_bits = 10U;
    }
    s_duty_max = (1U << resolution_bits) - 1U;
#ifdef ESP_PLATFORM
    ledc_timer_config_t t = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = (ledc_timer_bit_t)resolution_bits,
        .freq_hz = freq_hz != 0U ? freq_hz : 1000U,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    esp_err_t err = ledc_timer_config(&t);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    ledc_channel_config_t c = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = gpio_pin,
        .duty = 0,
        .hpoint = 0,
    };
    err = ledc_channel_config(&c);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
#else
    (void)gpio_pin;
    (void)freq_hz;
#endif
    s_init = true;
    IOT_LOGI(TAG, "pwm gpio=%d", gpio_pin);
    return IOT_OK;
}

iot_err_t iot_pwm_deinit(void)
{
    s_init = false;
    return IOT_OK;
}

iot_err_t iot_pwm_set_duty(uint32_t duty)
{
    if (!s_init) {
        return IOT_ERR_NOT_INIT;
    }
    if (duty > s_duty_max) {
        duty = s_duty_max;
    }
#ifdef ESP_PLATFORM
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    return iot_err_from_esp(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
#else
    return IOT_OK;
#endif
}
