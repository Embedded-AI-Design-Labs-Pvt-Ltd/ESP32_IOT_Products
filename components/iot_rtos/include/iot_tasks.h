/**
 * @file iot_tasks.h
 * @brief Named platform task table — single place for priorities and stacks.
 *
 * @sync ISR → binary semaphore → sensor task → sample queue → processing → telemetry queue → MQTT
 */
#ifndef IOT_TASKS_H
#define IOT_TASKS_H

#include "iot_err.h"
#include "iot_rtos.h"
#include "iot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_EVT_WIFI_IP          (1U << 0)
#define IOT_EVT_MQTT_UP          (1U << 1)
#define IOT_EVT_BLE_CONN         (1U << 2)
#define IOT_EVT_OTA_BUSY         (1U << 3)
#define IOT_EVT_FAULT            (1U << 4)
#define IOT_EVT_PROVISIONED      (1U << 5)
#define IOT_EVT_SHUTDOWN         (1U << 31)

typedef struct {
    iot_queue_t sample_q;     /**< Sensor task owns produce; processing owns consume. */
    iot_queue_t telemetry_q;  /**< Processing produce; MQTT consume. */
    iot_queue_t command_q;    /**< MQTT produce; application consume. */
    iot_sem_t sensor_isr_sem; /**< Given from ISR; taken by sensor task. */
    iot_mutex_t sample_lock;  /**< Protects last-sample cache. */
    iot_event_t system_evt;   /**< Cross-subsystem state bits. */
    iot_task_t sensor_task;
    iot_task_t proc_task;
    iot_task_t mqtt_task;
    iot_task_t cmd_task;
    iot_task_t diag_task;
    iot_task_t wdt_task;
    iot_task_t wifi_task;
    iot_task_t ble_task;
    iot_task_t storage_task;
    iot_task_t ota_task;
    iot_task_t app_task;
} iot_task_ctx_t;

iot_err_t iot_tasks_start(iot_task_ctx_t *ctx);
iot_task_ctx_t *iot_tasks_ctx(void);
void iot_tasks_request_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_TASKS_H */
