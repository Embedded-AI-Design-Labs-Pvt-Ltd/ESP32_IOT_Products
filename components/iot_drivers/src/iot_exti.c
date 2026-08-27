/**
 * @file iot_exti.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief GPIO ISR → binary semaphore. No logging, no malloc in ISR.
 */
#include "iot_exti.h"
#include "iot_log.h"

#ifdef ESP_PLATFORM
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#endif

#define IOT_EXTI_MAX 22

static iot_sem_t s_sem[IOT_EXTI_MAX];
static uint32_t s_count[IOT_EXTI_MAX];
static bool s_isr_svc;

#ifdef ESP_PLATFORM
static void IRAM_ATTR gpio_isr(void *arg)
{
    int pin = (int)(intptr_t)arg;
    int hp = 0;
    if ((pin >= 0) && (pin < IOT_EXTI_MAX)) {
        s_count[pin]++; /* single-writer ISR; task reads without lock for diagnostics */
        if (s_sem[pin] != NULL) {
            iot_sem_give_from_isr(s_sem[pin], &hp);
        }
    }
    if (hp != 0) {
        portYIELD_FROM_ISR();
    }
}
#endif

iot_err_t iot_exti_init(int pin, iot_exti_edge_t edge, iot_sem_t isr_sem)
{
    if ((pin < 0) || (pin >= IOT_EXTI_MAX) || (isr_sem == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    s_sem[pin] = isr_sem;
#ifdef ESP_PLATFORM
    if (!s_isr_svc) {
        esp_err_t e = gpio_install_isr_service(0);
        if (e != ESP_OK && e != ESP_ERR_INVALID_STATE) {
            return iot_err_from_esp(e);
        }
        s_isr_svc = true;
    }
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = (edge == IOT_EXTI_NEGEDGE) ? GPIO_INTR_NEGEDGE :
                     (edge == IOT_EXTI_ANYEDGE) ? GPIO_INTR_ANYEDGE : GPIO_INTR_POSEDGE,
    };
    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    err = gpio_isr_handler_add((gpio_num_t)pin, gpio_isr, (void *)(intptr_t)pin);
    return iot_err_from_esp(err);
#else
    (void)edge;
    s_isr_svc = true;
    return IOT_OK;
#endif
}

iot_err_t iot_exti_deinit(int pin)
{
#ifdef ESP_PLATFORM
    if ((pin >= 0) && (pin < IOT_EXTI_MAX)) {
        (void)gpio_isr_handler_remove((gpio_num_t)pin);
        s_sem[pin] = NULL;
    }
#else
    if ((pin >= 0) && (pin < IOT_EXTI_MAX)) {
        s_sem[pin] = NULL;
    }
#endif
    return IOT_OK;
}

uint32_t iot_exti_count(int pin)
{
    if ((pin < 0) || (pin >= IOT_EXTI_MAX)) {
        return 0U;
    }
    return s_count[pin];
}
