/**
 * @file iot_wdt.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Task watchdog. The dedicated wdt task is the subscribed task.
 */
#include "iot_wdt.h"
#include "iot_log.h"

#ifdef ESP_PLATFORM
#include "esp_task_wdt.h"
#endif

static const char *TAG = "iot_wdt";
static bool s_init;

iot_err_t iot_wdt_init(void)
{
#ifdef ESP_PLATFORM
    esp_task_wdt_config_t cfg = {
        .timeout_ms = 10000,
        .idle_core_mask = (1 << 0),
        .trigger_panic = true,
    };
    esp_err_t e = esp_task_wdt_reconfigure(&cfg);
    if (e == ESP_ERR_INVALID_STATE) {
        e = ESP_OK;
    }
    if (e != ESP_OK) {
        return iot_err_from_esp(e);
    }
#endif
    s_init = true;
    IOT_LOGI(TAG, "task wdt 10s panic=on");
    return IOT_OK;
}

iot_err_t iot_wdt_deinit(void)
{
    s_init = false;
    return IOT_OK;
}

iot_err_t iot_wdt_add_current(void)
{
#ifdef ESP_PLATFORM
    esp_err_t e = esp_task_wdt_add(NULL);
    if (e == ESP_ERR_INVALID_STATE) {
        return IOT_OK;
    }
    return iot_err_from_esp(e);
#else
    return IOT_OK;
#endif
}

iot_err_t iot_wdt_reset(void)
{
#ifdef ESP_PLATFORM
    return iot_err_from_esp(esp_task_wdt_reset());
#else
    return s_init ? IOT_OK : IOT_ERR_NOT_INIT;
#endif
}
