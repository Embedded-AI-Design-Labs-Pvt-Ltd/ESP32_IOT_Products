/**
 * @file iot_gpio.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief GPIO HAL. Mutex-owned; not ISR-safe except via iot_exti.
 */
#include "iot_gpio.h"
#include "iot_log.h"
#include "iot_rtos.h"

#ifdef ESP_PLATFORM
#include "driver/gpio.h"
#endif

static const char *TAG = "iot_gpio";
static iot_mutex_t s_lock;
static bool s_init;
static uint32_t s_out_shadow;

iot_err_t iot_gpio_init(void)
{
    if (s_init) {
        return IOT_ERR_ALREADY_INIT;
    }
    iot_err_t e = iot_mutex_create(&s_lock);
    if (IOT_FAIL(e)) {
        return e;
    }
    s_init = true;
    IOT_LOGI(TAG, "gpio init");
    return IOT_OK;
}

iot_err_t iot_gpio_deinit(void)
{
    if (!s_init) {
        return IOT_ERR_NOT_INIT;
    }
    iot_mutex_delete(s_lock);
    s_lock = NULL;
    s_init = false;
    return IOT_OK;
}

iot_err_t iot_gpio_config(int pin, iot_gpio_mode_t mode)
{
    if (!s_init || (pin < 0) || (pin > 21)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock, 1000U);
#ifdef ESP_PLATFORM
    gpio_config_t io = {0};
    io.pin_bit_mask = 1ULL << pin;
    io.intr_type = GPIO_INTR_DISABLE;
    if (mode == IOT_GPIO_OUT) {
        io.mode = GPIO_MODE_OUTPUT;
    } else {
        io.mode = GPIO_MODE_INPUT;
        io.pull_up_en = (mode == IOT_GPIO_IN_PULLUP) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
        io.pull_down_en = (mode == IOT_GPIO_IN_PULLDOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    }
    esp_err_t err = gpio_config(&io);
    iot_mutex_unlock(s_lock);
    return iot_err_from_esp(err);
#else
    (void)mode;
    iot_mutex_unlock(s_lock);
    return IOT_OK;
#endif
}

iot_err_t iot_gpio_write(int pin, bool level)
{
    if (!s_init || (pin < 0)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock, 1000U);
#ifdef ESP_PLATFORM
    esp_err_t err = gpio_set_level((gpio_num_t)pin, level ? 1 : 0);
    iot_mutex_unlock(s_lock);
    return iot_err_from_esp(err);
#else
    if (level) {
        s_out_shadow |= (1UL << pin);
    } else {
        s_out_shadow &= ~(1UL << pin);
    }
    iot_mutex_unlock(s_lock);
    return IOT_OK;
#endif
}

iot_err_t iot_gpio_read(int pin, bool *level)
{
    if (!s_init || (pin < 0) || (level == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock, 1000U);
#ifdef ESP_PLATFORM
    *level = gpio_get_level((gpio_num_t)pin) != 0;
#else
    *level = (s_out_shadow & (1UL << pin)) != 0U;
#endif
    iot_mutex_unlock(s_lock);
    return IOT_OK;
}

iot_err_t iot_gpio_toggle(int pin)
{
    bool cur = false;
    iot_err_t e = iot_gpio_read(pin, &cur);
    if (IOT_FAIL(e)) {
        return e;
    }
    return iot_gpio_write(pin, !cur);
}
