/**
 * @file iot_diag.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Health counters. ISR path only increments atomics.
 */
#include "iot_diag.h"
#include "iot_log.h"
#include "iot_board.h"
#include "iot_rtos.h"

#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

static const char *TAG = "iot_diag";
static volatile uint32_t s_wifi_fails;
static volatile uint32_t s_mqtt_disc;
static volatile uint32_t s_sensor_to;
static volatile uint32_t s_wdt;
static volatile uint32_t s_last;
static uint32_t s_boot_ms;

void iot_diag_isr_bump(iot_diag_id_t id)
{
    s_last = (uint32_t)id;
    if (id == DIAG_SENSOR_TIMEOUT) {
        s_sensor_to++;
    } else if (id == DIAG_WATCHDOG_RESET) {
        s_wdt++;
    }
}

void iot_diag_event(iot_diag_id_t id)
{
    s_last = (uint32_t)id;
    switch (id) {
    case DIAG_WIFI_CONNECT_FAIL:
        s_wifi_fails++;
        break;
    case DIAG_MQTT_DISCONNECT:
        s_mqtt_disc++;
        break;
    case DIAG_SENSOR_TIMEOUT:
        s_sensor_to++;
        break;
    case DIAG_WATCHDOG_RESET:
        s_wdt++;
        break;
    default:
        break;
    }
    if (id != DIAG_OK) {
        IOT_LOGW(TAG, "event 0x%04x", (unsigned)id);
    }
}

iot_err_t iot_diag_init(void)
{
    s_boot_ms = iot_millis();
#ifdef ESP_PLATFORM
    if (esp_reset_reason() == ESP_RST_TASK_WDT || esp_reset_reason() == ESP_RST_INT_WDT) {
        iot_diag_event(DIAG_WATCHDOG_RESET);
    }
#endif
    IOT_LOGI(TAG, "diag init reset=%s", iot_board_reset_reason());
    return IOT_OK;
}

iot_err_t iot_diag_snapshot(iot_diag_snapshot_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)memset(out, 0, sizeof(*out));
    out->uptime_s = (iot_millis() - s_boot_ms) / 1000U;
    out->wifi_fails = s_wifi_fails;
    out->mqtt_disconnects = s_mqtt_disc;
    out->sensor_timeouts = s_sensor_to;
    out->wdt_events = s_wdt;
    out->last_event = s_last;
#ifdef ESP_PLATFORM
    out->free_heap = (uint32_t)esp_get_free_heap_size();
    out->min_heap = (uint32_t)esp_get_minimum_free_heap_size();
#else
    out->free_heap = 256000U;
    out->min_heap = 200000U;
    out->cpu_pct = 12;
#endif
    return IOT_OK;
}

iot_err_t iot_diag_snapshot_json(char *buf, size_t len)
{
    iot_diag_snapshot_t s;
    iot_err_t e = iot_diag_snapshot(&s);
    if (IOT_FAIL(e) || (buf == NULL)) {
        return e != IOT_OK ? e : IOT_ERR_INVALID_ARG;
    }
    int n = snprintf(buf, len,
                     "{\"uptime\":%u,\"heap\":%u,\"min_heap\":%u,\"wifi_fail\":%u,"
                     "\"mqtt_disc\":%u,\"sensor_to\":%u,\"wdt\":%u,\"last\":%u}",
                     (unsigned)s.uptime_s, (unsigned)s.free_heap, (unsigned)s.min_heap,
                     (unsigned)s.wifi_fails, (unsigned)s.mqtt_disconnects,
                     (unsigned)s.sensor_timeouts, (unsigned)s.wdt_events, (unsigned)s.last_event);
    return (n < 0 || (size_t)n >= len) ? IOT_ERR_OVERFLOW : IOT_OK;
}
