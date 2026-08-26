/**
 * @file iot_timer.c
 * @brief GPTimer HAL. Alarm callback runs in ISR — must only GiveFromISR.
 */
#include "iot_timer.h"
#include "iot_log.h"

#ifdef ESP_PLATFORM
#include "driver/gptimer.h"

static gptimer_handle_t s_tmr;
static iot_hw_timer_cb_t s_cb;
static void *s_arg;

static bool IRAM_ATTR alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user)
{
    (void)timer;
    (void)edata;
    (void)user;
    if (s_cb != NULL) {
        s_cb(s_arg);
    }
    return true; /* re-enable alarm */
}
#endif

static const char *TAG = "iot_tmr";
static bool s_init;

iot_err_t iot_hw_timer_init(uint32_t period_us, iot_hw_timer_cb_t cb, void *arg)
{
    if ((cb == NULL) || (period_us == 0U)) {
        return IOT_ERR_INVALID_ARG;
    }
#ifdef ESP_PLATFORM
    s_cb = cb;
    s_arg = arg;
    gptimer_config_t cfg = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,
    };
    esp_err_t err = gptimer_new_timer(&cfg, &s_tmr);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    gptimer_event_callbacks_t cbs = {.on_alarm = alarm_cb};
    err = gptimer_register_event_callbacks(s_tmr, &cbs, NULL);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    gptimer_alarm_config_t ac = {
        .reload_count = 0,
        .alarm_count = period_us,
        .flags.auto_reload_on_alarm = true,
    };
    err = gptimer_set_alarm_action(s_tmr, &ac);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    err = gptimer_enable(s_tmr);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
#else
    (void)period_us;
    (void)cb;
    (void)arg;
#endif
    s_init = true;
    IOT_LOGI(TAG, "gptimer period_us=%u", (unsigned)period_us);
    return IOT_OK;
}

iot_err_t iot_hw_timer_deinit(void)
{
#ifdef ESP_PLATFORM
    if (s_tmr != NULL) {
        (void)gptimer_disable(s_tmr);
        (void)gptimer_del_timer(s_tmr);
        s_tmr = NULL;
    }
#endif
    s_init = false;
    return IOT_OK;
}

iot_err_t iot_hw_timer_start(void)
{
#ifdef ESP_PLATFORM
    if (s_tmr == NULL) {
        return IOT_ERR_NOT_INIT;
    }
    return iot_err_from_esp(gptimer_start(s_tmr));
#else
    return s_init ? IOT_OK : IOT_ERR_NOT_INIT;
#endif
}

iot_err_t iot_hw_timer_stop(void)
{
#ifdef ESP_PLATFORM
    if (s_tmr == NULL) {
        return IOT_ERR_NOT_INIT;
    }
    return iot_err_from_esp(gptimer_stop(s_tmr));
#else
    return IOT_OK;
#endif
}
