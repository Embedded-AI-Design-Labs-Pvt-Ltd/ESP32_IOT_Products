/**
 * @file iot_ota.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief HTTPS OTA using esp_https_ota. Abort leaves previous slot bootable.
 *
 * @syscalls esp_https_ota, esp_ota_mark_app_valid_cancel_rollback, esp_task_wdt_reset
 */
#include "iot_ota.h"
#include "iot_log.h"
#include "iot_wdt.h"
#include "iot_tasks.h"
#include "iot_rtos.h"

#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

static const char *TAG = "iot_ota";
static iot_ota_state_t s_state = IOT_OTA_IDLE;
static char s_url[256];

#ifdef ESP_PLATFORM
static void ota_task(void *arg)
{
    (void)arg;
    iot_task_ctx_t *ctx = iot_tasks_ctx();
    if ((ctx != NULL) && (ctx->system_evt != NULL)) {
        iot_event_set(ctx->system_evt, IOT_EVT_OTA_BUSY);
    }
    s_state = IOT_OTA_DOWNLOADING;
    esp_http_client_config_t http = {
        .url = s_url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 15000,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t oc = {
        .http_config = &http,
    };
    esp_https_ota_handle_t h = NULL;
    esp_err_t err = esp_https_ota_begin(&oc, &h);
    if (err != ESP_OK) {
        s_state = IOT_OTA_FAIL;
        IOT_LOGE(TAG, "begin failed %s", esp_err_to_name(err));
        goto done;
    }
    while (1) {
        (void)iot_wdt_reset();
        err = esp_https_ota_perform(h);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
    }
    if (esp_https_ota_is_complete_data_received(h) && err == ESP_OK) {
        s_state = IOT_OTA_VERIFY;
        err = esp_https_ota_finish(h);
        if (err == ESP_OK) {
            s_state = IOT_OTA_REBOOT;
            IOT_LOGI(TAG, "ota ok — reboot");
            iot_task_delay_ms(500U);
            esp_restart();
        } else {
            s_state = IOT_OTA_FAIL;
        }
    } else {
        (void)esp_https_ota_abort(h);
        s_state = IOT_OTA_FAIL;
        IOT_LOGE(TAG, "ota abort — previous slot remains bootable");
    }
done:
    if ((ctx != NULL) && (ctx->system_evt != NULL)) {
        iot_event_clear(ctx->system_evt, IOT_EVT_OTA_BUSY);
    }
    vTaskDelete(NULL);
}
#endif

iot_err_t iot_ota_init(void)
{
    s_state = IOT_OTA_IDLE;
#ifdef ESP_PLATFORM
    const esp_partition_t *run = esp_ota_get_running_partition();
    IOT_LOGI(TAG, "running ota subtype=%d", run != NULL ? run->subtype : -1);
#endif
    return IOT_OK;
}

iot_err_t iot_ota_start(const char *url)
{
    if ((url == NULL) || (strncmp(url, "https://", 8) != 0)) {
        return IOT_ERR_DENIED;
    }
    if (s_state == IOT_OTA_DOWNLOADING) {
        return IOT_ERR_BUSY;
    }
    (void)memset(s_url, 0, sizeof(s_url));
    (void)strncpy(s_url, url, sizeof(s_url) - 1U);
#ifdef ESP_PLATFORM
    iot_task_t t;
    return iot_task_create("ota", ota_task, NULL, 8192U, IOT_PRIO_OTA, &t);
#else
    s_state = IOT_OTA_VERIFY;
    IOT_LOGI(TAG, "sim ota url=%s", s_url);
    return IOT_OK;
#endif
}

iot_ota_state_t iot_ota_state(void)
{
    return s_state;
}

iot_err_t iot_ota_mark_valid(void)
{
#ifdef ESP_PLATFORM
    return iot_err_from_esp(esp_ota_mark_app_valid_cancel_rollback());
#else
    return IOT_OK;
#endif
}
