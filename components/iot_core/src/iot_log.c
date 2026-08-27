/**
 * @file iot_log.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Logging backend.
 *
 * @syscalls vprintf / esp_log_write
 * @thread any task; not ISR-safe
 */
#include "iot_log.h"

#include <stdarg.h>
#include <stdio.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
#endif

static iot_log_level_t s_level = IOT_LOG_INFO;

void iot_log_set_level(iot_log_level_t level)
{
    s_level = level;
#ifdef ESP_PLATFORM
    esp_log_level_t el = ESP_LOG_INFO;
    if (level == IOT_LOG_ERROR) {
        el = ESP_LOG_ERROR;
    } else if (level == IOT_LOG_WARN) {
        el = ESP_LOG_WARN;
    } else if (level == IOT_LOG_DEBUG) {
        el = ESP_LOG_DEBUG;
    } else if (level == IOT_LOG_VERBOSE) {
        el = ESP_LOG_VERBOSE;
    } else if (level == IOT_LOG_NONE) {
        el = ESP_LOG_NONE;
    }
    esp_log_level_set("*", el);
#endif
}

iot_log_level_t iot_log_get_level(void)
{
    return s_level;
}

void iot_log_write(iot_log_level_t level, const char *tag, const char *fmt, ...)
{
    if ((tag == NULL) || (fmt == NULL) || (level > s_level) || (level == IOT_LOG_NONE)) {
        return;
    }

    va_list args;
    va_start(args, fmt);
#ifdef ESP_PLATFORM
    esp_log_level_t el = ESP_LOG_INFO;
    if (level == IOT_LOG_ERROR) {
        el = ESP_LOG_ERROR;
    } else if (level == IOT_LOG_WARN) {
        el = ESP_LOG_WARN;
    } else if (level == IOT_LOG_DEBUG) {
        el = ESP_LOG_DEBUG;
    } else if (level == IOT_LOG_VERBOSE) {
        el = ESP_LOG_VERBOSE;
    }
    esp_log_writev(el, tag, fmt, args);
    esp_log_write(el, tag, "\n");
#else
    static const char *names[] = {"N", "E", "W", "I", "D", "V"};
    (void)printf("[%s] %s: ", names[level], tag);
    (void)vprintf(fmt, args);
    (void)printf("\n");
#endif
    va_end(args);
}
