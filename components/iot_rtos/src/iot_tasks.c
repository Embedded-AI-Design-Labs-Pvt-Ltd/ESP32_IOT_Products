/**
 * @file iot_tasks.c
 * @brief Platform task set and ISR→task→queue data path.
 *
 * Sequence:
 *   GPIO/ADC ISR --GiveFromISR--> sensor_isr_sem
 *   Sensor task  --queue_send---> sample_q
 *   Proc task    --queue_send---> telemetry_q  (MQTT consumes)
 *   MQTT inbound --queue_send---> command_q
 *   Cmd task     --profile hook-> HAL actuator
 *
 * @ownership Queues and semaphores are owned by this module until shutdown.
 */
#include "iot_tasks.h"
#include "iot_log.h"

#include <string.h>

static const char *TAG = "iot_tasks";
static iot_task_ctx_t s_ctx;
static bool s_started;
static volatile bool s_shutdown;

typedef iot_err_t (*iot_sensor_hook_t)(iot_telemetry_msg_t *out);
typedef iot_err_t (*iot_proc_hook_t)(const iot_telemetry_msg_t *in, iot_telemetry_msg_t *out);
typedef iot_err_t (*iot_cmd_hook_t)(const iot_command_msg_t *cmd);

static iot_sensor_hook_t s_sensor_hook;
static iot_proc_hook_t s_proc_hook;
static iot_cmd_hook_t s_cmd_hook;
static uint32_t s_seq;

void iot_tasks_set_hooks(iot_sensor_hook_t sensor, iot_proc_hook_t proc, iot_cmd_hook_t cmd)
{
    s_sensor_hook = sensor;
    s_proc_hook = proc;
    s_cmd_hook = cmd;
}

iot_task_ctx_t *iot_tasks_ctx(void)
{
    return &s_ctx;
}

void iot_tasks_request_shutdown(void)
{
    s_shutdown = true;
    if (s_ctx.system_evt != NULL) {
        iot_event_set(s_ctx.system_evt, IOT_EVT_SHUTDOWN);
    }
}

static void sensor_task(void *arg)
{
    (void)arg;
    iot_telemetry_msg_t msg;
    IOT_LOGI(TAG, "sensor task start");
    for (;;) {
        if (s_shutdown) {
            break;
        }
        /* Block on ISR (or periodic poll if no ISR fired). */
        (void)iot_sem_take(s_ctx.sensor_isr_sem, 200U);
        (void)memset(&msg, 0, sizeof(msg));
        msg.seq = ++s_seq;
        msg.unix_ms = (int64_t)iot_millis();
        iot_err_t e = IOT_OK;
        if (s_sensor_hook != NULL) {
            e = s_sensor_hook(&msg);
        } else {
            msg.payload_len = 2U;
            msg.payload[0] = '{';
            msg.payload[1] = '}';
        }
        if (IOT_SUCCESS(e)) {
            if (iot_queue_send(s_ctx.sample_q, &msg, 10U) != IOT_OK) {
                IOT_LOGW(TAG, "sample queue full — drop seq=%u", (unsigned)msg.seq);
            }
        }
    }
}

static void proc_task(void *arg)
{
    (void)arg;
    iot_telemetry_msg_t in;
    iot_telemetry_msg_t out;
    IOT_LOGI(TAG, "processing task start");
    for (;;) {
        if (s_shutdown) {
            break;
        }
        if (iot_queue_recv(s_ctx.sample_q, &in, 250U) != IOT_OK) {
            continue;
        }
        out = in;
        if (s_proc_hook != NULL) {
            (void)s_proc_hook(&in, &out);
        }
        if (iot_queue_send(s_ctx.telemetry_q, &out, 50U) != IOT_OK) {
            IOT_LOGW(TAG, "telemetry queue full");
        }
    }
}

static void cmd_task(void *arg)
{
    (void)arg;
    iot_command_msg_t cmd;
    IOT_LOGI(TAG, "command task start");
    for (;;) {
        if (s_shutdown) {
            break;
        }
        if (iot_queue_recv(s_ctx.command_q, &cmd, 250U) != IOT_OK) {
            continue;
        }
        if (s_cmd_hook != NULL) {
            (void)s_cmd_hook(&cmd);
        }
    }
}

static void diag_task(void *arg)
{
    (void)arg;
    IOT_LOGI(TAG, "diagnostics task start");
    for (;;) {
        if (s_shutdown) {
            break;
        }
        iot_task_delay_ms(5000U);
        /* Snapshot published by iot_diag; this task keeps the WDT-visible heartbeat. */
    }
}

static void wdt_task(void *arg)
{
    (void)arg;
    IOT_LOGI(TAG, "watchdog monitor task start");
    for (;;) {
        if (s_shutdown) {
            break;
        }
#ifdef ESP_PLATFORM
        extern iot_err_t iot_wdt_reset(void);
        (void)iot_wdt_reset();
#endif
        iot_task_delay_ms(2000U);
    }
}

iot_err_t iot_tasks_start(iot_task_ctx_t *ctx)
{
    if (s_started) {
        return IOT_ERR_ALREADY_INIT;
    }
    iot_err_t e;
    e = iot_queue_create(16U, (uint32_t)sizeof(iot_telemetry_msg_t), &s_ctx.sample_q);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_queue_create(16U, (uint32_t)sizeof(iot_telemetry_msg_t), &s_ctx.telemetry_q);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_queue_create(8U, (uint32_t)sizeof(iot_command_msg_t), &s_ctx.command_q);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_sem_binary_create(&s_ctx.sensor_isr_sem);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_mutex_create(&s_ctx.sample_lock);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_event_create(&s_ctx.system_evt);
    if (IOT_FAIL(e)) {
        return e;
    }

    /* Kick the first sample without waiting for an ISR (safe on sim and boot). */
    iot_sem_give(s_ctx.sensor_isr_sem);

    e = iot_task_create("sensor", sensor_task, NULL, 4096U, IOT_PRIO_SENSOR, &s_ctx.sensor_task);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_task_create("proc", proc_task, NULL, 4096U, IOT_PRIO_PROC, &s_ctx.proc_task);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_task_create("cmd", cmd_task, NULL, 4096U, IOT_PRIO_CMD, &s_ctx.cmd_task);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_task_create("diag", diag_task, NULL, 3072U, IOT_PRIO_DIAG, &s_ctx.diag_task);
    if (IOT_FAIL(e)) {
        return e;
    }
    e = iot_task_create("wdt", wdt_task, NULL, 2048U, IOT_PRIO_WDT, &s_ctx.wdt_task);
    if (IOT_FAIL(e)) {
        return e;
    }

    s_started = true;
    if (ctx != NULL) {
        *ctx = s_ctx;
    }
    IOT_LOGI(TAG, "task architecture started");
    return IOT_OK;
}
