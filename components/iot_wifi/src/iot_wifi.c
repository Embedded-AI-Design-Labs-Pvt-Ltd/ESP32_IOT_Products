/**
 * @file iot_wifi.c
 * @brief STA connection manager. ESP-IDF event loop drives the state machine.
 *
 * @thread wifi_mgr task + Wi-Fi event task (IDF)
 * @syscalls esp_wifi_*, esp_netif_*, esp_event_*
 */
#include "iot_wifi.h"
#include "iot_log.h"
#include "iot_rtos.h"
#include "iot_tasks.h"

#include <string.h>
#include <stdio.h>

#ifdef ESP_PLATFORM
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

static const char *TAG = "iot_wifi";
static iot_wifi_state_t s_state = WIFI_DISCONNECTED;
static iot_mutex_t s_lock;
static iot_wifi_status_t s_status;
static uint32_t s_backoff_ms = 1000U;
static char s_ssid[33];
static char s_pass[65];

static void set_state(iot_wifi_state_t st)
{
    (void)iot_mutex_lock(s_lock, 1000U);
    s_state = st;
    s_status.state = st;
    iot_mutex_unlock(s_lock);
    iot_task_ctx_t *ctx = iot_tasks_ctx();
    if ((ctx != NULL) && (ctx->system_evt != NULL)) {
        if (st == WIFI_IP_READY) {
            iot_event_set(ctx->system_evt, IOT_EVT_WIFI_IP);
        } else {
            iot_event_clear(ctx->system_evt, IOT_EVT_WIFI_IP | IOT_EVT_MQTT_UP);
        }
    }
}

#ifdef ESP_PLATFORM
static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    (void)data;
    if (base == WIFI_EVENT) {
        if (id == WIFI_EVENT_STA_START) {
            set_state(WIFI_CONNECTING);
            (void)esp_wifi_connect();
        } else if (id == WIFI_EVENT_STA_CONNECTED) {
            set_state(WIFI_CONNECTED);
            s_backoff_ms = 1000U;
        } else if (id == WIFI_EVENT_STA_DISCONNECTED) {
            s_status.retry_count++;
            set_state(WIFI_RECONNECTING);
            /* Exponential backoff, cap 30 s. Event handler must not block long. */
            uint32_t delay = s_backoff_ms;
            if (s_backoff_ms < 30000U) {
                s_backoff_ms *= 2U;
            }
            (void)esp_timer_get_time();
            (void)vTaskDelay(pdMS_TO_TICKS(delay > 50U ? 50U : delay));
            (void)esp_wifi_connect();
            set_state(WIFI_CONNECTING);
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ev = (ip_event_got_ip_t *)data;
        (void)snprintf(s_status.ip, sizeof(s_status.ip), IPSTR, IP2STR(&ev->ip_info.ip));
        set_state(WIFI_IP_READY);
        IOT_LOGI(TAG, "ip %s", s_status.ip);
    }
}
#endif

iot_err_t iot_wifi_init(void)
{
    iot_err_t e = iot_mutex_create(&s_lock);
    if (IOT_FAIL(e)) {
        return e;
    }
    (void)memset(&s_status, 0, sizeof(s_status));
#ifdef ESP_PLATFORM
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    (void)esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
#endif
    s_state = WIFI_DISCONNECTED;
    IOT_LOGI(TAG, "wifi mgr init");
    return IOT_OK;
}

iot_err_t iot_wifi_start(const char *ssid, const char *pass)
{
    if ((ssid == NULL) || (ssid[0] == '\0')) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)memset(s_ssid, 0, sizeof(s_ssid));
    (void)memset(s_pass, 0, sizeof(s_pass));
    (void)strncpy(s_ssid, ssid, sizeof(s_ssid) - 1U);
    if (pass != NULL) {
        (void)strncpy(s_pass, pass, sizeof(s_pass) - 1U);
    }
#ifdef ESP_PLATFORM
    wifi_config_t wc = {0};
    (void)strncpy((char *)wc.sta.ssid, s_ssid, sizeof(wc.sta.ssid));
    (void)strncpy((char *)wc.sta.password, s_pass, sizeof(wc.sta.password));
    wc.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wc.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wc);
    if (err != ESP_OK) {
        set_state(WIFI_ERROR);
        return iot_err_from_esp(err);
    }
    set_state(WIFI_CONNECTING);
    err = esp_wifi_start();
    return iot_err_from_esp(err);
#else
    set_state(WIFI_IP_READY);
    (void)memcpy(s_status.ip, "127.0.0.1", 10);
    IOT_LOGI(TAG, "sim wifi ip-ready ssid=%s", s_ssid);
    return IOT_OK;
#endif
}

iot_err_t iot_wifi_stop(void)
{
#ifdef ESP_PLATFORM
    (void)esp_wifi_disconnect();
    (void)esp_wifi_stop();
#endif
    set_state(WIFI_DISCONNECTED);
    return IOT_OK;
}

iot_wifi_state_t iot_wifi_state(void)
{
    return s_state;
}

iot_err_t iot_wifi_get_status(iot_wifi_status_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
#ifdef ESP_PLATFORM
    wifi_ap_record_t ap;
    if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
        s_status.rssi = ap.rssi;
    }
#endif
    (void)iot_mutex_lock(s_lock, 100U);
    *out = s_status;
    iot_mutex_unlock(s_lock);
    return IOT_OK;
}
