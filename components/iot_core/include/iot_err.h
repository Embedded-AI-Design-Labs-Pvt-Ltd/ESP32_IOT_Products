/**
 * @file iot_err.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Platform-wide error codes and propagation helpers.
 *
 * @ownership All layers return iot_err_t. ESP-IDF esp_err_t is converted at the HAL boundary.
 * @thread_safety Pure value type; no shared state.
 */
#ifndef IOT_ERR_H
#define IOT_ERR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Signed error type. Zero is success; negative values are failures. */
typedef int32_t iot_err_t;

#define IOT_OK                  ((iot_err_t)0)
#define IOT_ERR_FAIL            ((iot_err_t)-1)
#define IOT_ERR_INVALID_ARG     ((iot_err_t)-2)
#define IOT_ERR_NO_MEM          ((iot_err_t)-3)
#define IOT_ERR_TIMEOUT         ((iot_err_t)-4)
#define IOT_ERR_BUSY            ((iot_err_t)-5)
#define IOT_ERR_NOT_FOUND       ((iot_err_t)-6)
#define IOT_ERR_NOT_SUPPORTED   ((iot_err_t)-7)
#define IOT_ERR_INVALID_STATE   ((iot_err_t)-8)
#define IOT_ERR_IO              ((iot_err_t)-9)
#define IOT_ERR_CRC             ((iot_err_t)-10)
#define IOT_ERR_AUTH            ((iot_err_t)-11)
#define IOT_ERR_DENIED          ((iot_err_t)-12)
#define IOT_ERR_OVERFLOW        ((iot_err_t)-13)
#define IOT_ERR_AGAIN           ((iot_err_t)-14)
#define IOT_ERR_HW              ((iot_err_t)-15)
#define IOT_ERR_NOT_INIT        ((iot_err_t)-16)
#define IOT_ERR_ALREADY_INIT    ((iot_err_t)-17)
#define IOT_ERR_QUEUE_FULL      ((iot_err_t)-18)
#define IOT_ERR_QUEUE_EMPTY     ((iot_err_t)-19)
#define IOT_ERR_PROTOCOL        ((iot_err_t)-20)
#define IOT_ERR_TLS             ((iot_err_t)-21)
#define IOT_ERR_OTA             ((iot_err_t)-22)
#define IOT_ERR_WATCHDOG        ((iot_err_t)-23)
#define IOT_ERR_ISR             ((iot_err_t)-24)

#define IOT_SUCCESS(e)  ((e) == IOT_OK)
#define IOT_FAIL(e)     ((e) != IOT_OK)

/**
 * @brief Convert ESP-IDF esp_err_t to iot_err_t at the HAL boundary.
 * @param esp_code ESP-IDF error (or 0 on host).
 */
iot_err_t iot_err_from_esp(int esp_code);

/** @brief Human-readable error name for diagnostics and HTML docs. */
const char *iot_err_to_name(iot_err_t err);

#ifdef __cplusplus
}
#endif

#endif /* IOT_ERR_H */
