/**
 * @file iot_log.h
 * @brief Diagnostic logging facade. Maps to ESP_LOG on target, stdout on host.
 *
 * @isr_safety Do not call from ISR. Use iot_diag_isr_bump() instead.
 */
#ifndef IOT_LOG_H
#define IOT_LOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_LOG_NONE = 0,
    IOT_LOG_ERROR,
    IOT_LOG_WARN,
    IOT_LOG_INFO,
    IOT_LOG_DEBUG,
    IOT_LOG_VERBOSE
} iot_log_level_t;

void iot_log_set_level(iot_log_level_t level);
iot_log_level_t iot_log_get_level(void);

void iot_log_write(iot_log_level_t level, const char *tag, const char *fmt, ...);

#define IOT_LOGE(tag, fmt, ...) iot_log_write(IOT_LOG_ERROR, (tag), (fmt), ##__VA_ARGS__)
#define IOT_LOGW(tag, fmt, ...) iot_log_write(IOT_LOG_WARN, (tag), (fmt), ##__VA_ARGS__)
#define IOT_LOGI(tag, fmt, ...) iot_log_write(IOT_LOG_INFO, (tag), (fmt), ##__VA_ARGS__)
#define IOT_LOGD(tag, fmt, ...) iot_log_write(IOT_LOG_DEBUG, (tag), (fmt), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* IOT_LOG_H */
