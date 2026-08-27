/**
 * @file iot_rtos.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief OS abstraction over FreeRTOS (target) and Win32/POSIX (host simulation).
 *
 * @ownership Callers own the objects they create and must destroy them.
 * @isr Only *_from_isr APIs are ISR-safe.
 */
#ifndef IOT_RTOS_H
#define IOT_RTOS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *iot_task_t;
typedef void *iot_mutex_t;
typedef void *iot_sem_t;
typedef void *iot_queue_t;
typedef void *iot_event_t;
typedef void *iot_timer_t;

typedef void (*iot_task_fn_t)(void *arg);
typedef void (*iot_timer_cb_t)(iot_timer_t timer, void *arg);

typedef enum {
    IOT_PRIO_IDLE = 1,
    IOT_PRIO_DIAG = 3,
    IOT_PRIO_OTA = 4,
    IOT_PRIO_STORAGE = 5,
    IOT_PRIO_APP = 6,
    IOT_PRIO_PROC = 7,
    IOT_PRIO_MQTT = 8,
    IOT_PRIO_WIFI = 9,
    IOT_PRIO_BLE = 10,
    IOT_PRIO_CMD = 11,
    IOT_PRIO_SENSOR = 12,
    IOT_PRIO_WDT = 14
} iot_task_prio_t;

iot_err_t iot_rtos_init(void);

iot_err_t iot_task_create(const char *name, iot_task_fn_t fn, void *arg, uint32_t stack_bytes,
                          iot_task_prio_t prio, iot_task_t *out);
void iot_task_delay_ms(uint32_t ms);
void iot_task_notify_give(iot_task_t task);
uint32_t iot_task_notify_take(uint32_t timeout_ms);
void iot_task_notify_give_from_isr(iot_task_t task, int *hp_woken);

iot_err_t iot_mutex_create(iot_mutex_t *out);
void iot_mutex_delete(iot_mutex_t m);
iot_err_t iot_mutex_lock(iot_mutex_t m, uint32_t timeout_ms);
void iot_mutex_unlock(iot_mutex_t m);

iot_err_t iot_sem_binary_create(iot_sem_t *out);
iot_err_t iot_sem_counting_create(uint32_t max_count, uint32_t init, iot_sem_t *out);
void iot_sem_delete(iot_sem_t s);
iot_err_t iot_sem_take(iot_sem_t s, uint32_t timeout_ms);
void iot_sem_give(iot_sem_t s);
void iot_sem_give_from_isr(iot_sem_t s, int *hp_woken);

iot_err_t iot_queue_create(uint32_t length, uint32_t item_size, iot_queue_t *out);
void iot_queue_delete(iot_queue_t q);
iot_err_t iot_queue_send(iot_queue_t q, const void *item, uint32_t timeout_ms);
iot_err_t iot_queue_send_from_isr(iot_queue_t q, const void *item, int *hp_woken);
iot_err_t iot_queue_recv(iot_queue_t q, void *item, uint32_t timeout_ms);
uint32_t iot_queue_waiting(iot_queue_t q);

iot_err_t iot_event_create(iot_event_t *out);
void iot_event_delete(iot_event_t e);
void iot_event_set(iot_event_t e, uint32_t bits);
void iot_event_clear(iot_event_t e, uint32_t bits);
uint32_t iot_event_wait(iot_event_t e, uint32_t bits, bool clear, bool wait_all, uint32_t timeout_ms);

iot_err_t iot_timer_create(const char *name, uint32_t period_ms, bool periodic, iot_timer_cb_t cb,
                           void *arg, iot_timer_t *out);
iot_err_t iot_timer_start(iot_timer_t t);
iot_err_t iot_timer_stop(iot_timer_t t);
void iot_timer_delete(iot_timer_t t);

void iot_enter_critical(void);
void iot_exit_critical(void);

uint32_t iot_millis(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_RTOS_H */
