/**
 * @file iot_diag.h
 * @brief Diagnostic event IDs, counters, and health snapshot.
 */
#ifndef IOT_DIAG_H
#define IOT_DIAG_H

#include "iot_err.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DIAG_OK = 0,
    DIAG_WIFI_CONNECT_FAIL = 0x1001,
    DIAG_MQTT_DISCONNECT = 0x1002,
    DIAG_SENSOR_TIMEOUT = 0x1003,
    DIAG_UART_ERROR = 0x1004,
    DIAG_I2C_ERROR = 0x1005,
    DIAG_SPI_ERROR = 0x1006,
    DIAG_WATCHDOG_RESET = 0x1007,
    DIAG_OTA_FAIL = 0x1008,
    DIAG_HEAP_LOW = 0x1009,
    DIAG_STACK_LOW = 0x100A,
    DIAG_CMD_REJECT = 0x100B,
    DIAG_BLE_DISCONNECT = 0x100C
} iot_diag_id_t;

typedef struct {
    uint32_t uptime_s;
    uint32_t free_heap;
    uint32_t min_heap;
    uint32_t wifi_fails;
    uint32_t mqtt_disconnects;
    uint32_t sensor_timeouts;
    uint32_t wdt_events;
    uint32_t last_event;
    int8_t cpu_pct;
} iot_diag_snapshot_t;

iot_err_t iot_diag_init(void);
void iot_diag_event(iot_diag_id_t id);
void iot_diag_isr_bump(iot_diag_id_t id);
iot_err_t iot_diag_snapshot(iot_diag_snapshot_t *out);
iot_err_t iot_diag_snapshot_json(char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* IOT_DIAG_H */
