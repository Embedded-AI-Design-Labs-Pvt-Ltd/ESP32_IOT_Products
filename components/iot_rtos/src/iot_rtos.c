/**
 * @file iot_rtos.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief FreeRTOS wrappers (ESP32-C3) and Win32/POSIX simulation (host).
 *
 * Demonstrates: xTaskCreate, queues, mutex, binary/counting semaphore,
 * event groups, task notifications, software timers, critical sections.
 *
 * @syscalls FreeRTOS kernel / CreateThread / pthread_create
 * @thread creator context unless noted FromISR
 */
#include "iot_rtos.h"
#include "iot_log.h"

#include <string.h>
#include <stdlib.h>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "esp_timer.h"

iot_err_t iot_rtos_init(void)
{
    return IOT_OK;
}

iot_err_t iot_task_create(const char *name, iot_task_fn_t fn, void *arg, uint32_t stack_bytes,
                          iot_task_prio_t prio, iot_task_t *out)
{
    if ((fn == NULL) || (out == NULL) || (stack_bytes < 1024U)) {
        return IOT_ERR_INVALID_ARG;
    }
    TaskHandle_t h = NULL;
    /* stack_bytes → words for FreeRTOS on RISC-V (32-bit). */
    const uint32_t words = stack_bytes / sizeof(StackType_t);
    if (xTaskCreate(fn, name != NULL ? name : "iot", words, arg, (UBaseType_t)prio, &h) != pdPASS) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_task_t)h;
    return IOT_OK;
}

void iot_task_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms == 0U ? 1U : ms));
}

void iot_task_notify_give(iot_task_t task)
{
    if (task != NULL) {
        (void)xTaskNotifyGive((TaskHandle_t)task);
    }
}

uint32_t iot_task_notify_take(uint32_t timeout_ms)
{
    return ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms));
}

void iot_task_notify_give_from_isr(iot_task_t task, int *hp_woken)
{
    BaseType_t woken = pdFALSE;
    if (task != NULL) {
        vTaskNotifyGiveFromISR((TaskHandle_t)task, &woken);
    }
    if (hp_woken != NULL) {
        *hp_woken = (int)woken;
    }
}

iot_err_t iot_mutex_create(iot_mutex_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    SemaphoreHandle_t m = xSemaphoreCreateMutex();
    if (m == NULL) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_mutex_t)m;
    return IOT_OK;
}

void iot_mutex_delete(iot_mutex_t m)
{
    if (m != NULL) {
        vSemaphoreDelete((SemaphoreHandle_t)m);
    }
}

iot_err_t iot_mutex_lock(iot_mutex_t m, uint32_t timeout_ms)
{
    if (m == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (xSemaphoreTake((SemaphoreHandle_t)m, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        return IOT_ERR_TIMEOUT;
    }
    return IOT_OK;
}

void iot_mutex_unlock(iot_mutex_t m)
{
    if (m != NULL) {
        (void)xSemaphoreGive((SemaphoreHandle_t)m);
    }
}

iot_err_t iot_sem_binary_create(iot_sem_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    SemaphoreHandle_t s = xSemaphoreCreateBinary();
    if (s == NULL) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_sem_t)s;
    return IOT_OK;
}

iot_err_t iot_sem_counting_create(uint32_t max_count, uint32_t init, iot_sem_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    SemaphoreHandle_t s = xSemaphoreCreateCounting(max_count, init);
    if (s == NULL) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_sem_t)s;
    return IOT_OK;
}

void iot_sem_delete(iot_sem_t s)
{
    if (s != NULL) {
        vSemaphoreDelete((SemaphoreHandle_t)s);
    }
}

iot_err_t iot_sem_take(iot_sem_t s, uint32_t timeout_ms)
{
    if (s == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (xSemaphoreTake((SemaphoreHandle_t)s, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        return IOT_ERR_TIMEOUT;
    }
    return IOT_OK;
}

void iot_sem_give(iot_sem_t s)
{
    if (s != NULL) {
        (void)xSemaphoreGive((SemaphoreHandle_t)s);
    }
}

void iot_sem_give_from_isr(iot_sem_t s, int *hp_woken)
{
    BaseType_t woken = pdFALSE;
    if (s != NULL) {
        (void)xSemaphoreGiveFromISR((SemaphoreHandle_t)s, &woken);
    }
    if (hp_woken != NULL) {
        *hp_woken = (int)woken;
    }
}

iot_err_t iot_queue_create(uint32_t length, uint32_t item_size, iot_queue_t *out)
{
    if ((out == NULL) || (length == 0U) || (item_size == 0U)) {
        return IOT_ERR_INVALID_ARG;
    }
    QueueHandle_t q = xQueueCreate(length, item_size);
    if (q == NULL) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_queue_t)q;
    return IOT_OK;
}

void iot_queue_delete(iot_queue_t q)
{
    if (q != NULL) {
        vQueueDelete((QueueHandle_t)q);
    }
}

iot_err_t iot_queue_send(iot_queue_t q, const void *item, uint32_t timeout_ms)
{
    if ((q == NULL) || (item == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    if (xQueueSend((QueueHandle_t)q, item, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        return IOT_ERR_QUEUE_FULL;
    }
    return IOT_OK;
}

iot_err_t iot_queue_send_from_isr(iot_queue_t q, const void *item, int *hp_woken)
{
    BaseType_t woken = pdFALSE;
    if ((q == NULL) || (item == NULL)) {
        return IOT_ERR_ISR;
    }
    if (xQueueSendFromISR((QueueHandle_t)q, item, &woken) != pdTRUE) {
        if (hp_woken != NULL) {
            *hp_woken = (int)woken;
        }
        return IOT_ERR_QUEUE_FULL;
    }
    if (hp_woken != NULL) {
        *hp_woken = (int)woken;
    }
    return IOT_OK;
}

iot_err_t iot_queue_recv(iot_queue_t q, void *item, uint32_t timeout_ms)
{
    if ((q == NULL) || (item == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    if (xQueueReceive((QueueHandle_t)q, item, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        return IOT_ERR_TIMEOUT;
    }
    return IOT_OK;
}

uint32_t iot_queue_waiting(iot_queue_t q)
{
    if (q == NULL) {
        return 0U;
    }
    return (uint32_t)uxQueueMessagesWaiting((QueueHandle_t)q);
}

iot_err_t iot_event_create(iot_event_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    EventGroupHandle_t e = xEventGroupCreate();
    if (e == NULL) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_event_t)e;
    return IOT_OK;
}

void iot_event_delete(iot_event_t e)
{
    if (e != NULL) {
        vEventGroupDelete((EventGroupHandle_t)e);
    }
}

void iot_event_set(iot_event_t e, uint32_t bits)
{
    if (e != NULL) {
        (void)xEventGroupSetBits((EventGroupHandle_t)e, bits);
    }
}

void iot_event_clear(iot_event_t e, uint32_t bits)
{
    if (e != NULL) {
        (void)xEventGroupClearBits((EventGroupHandle_t)e, bits);
    }
}

uint32_t iot_event_wait(iot_event_t e, uint32_t bits, bool clear, bool wait_all, uint32_t timeout_ms)
{
    if (e == NULL) {
        return 0U;
    }
    return (uint32_t)xEventGroupWaitBits((EventGroupHandle_t)e, bits, clear ? pdTRUE : pdFALSE,
                                         wait_all ? pdTRUE : pdFALSE, pdMS_TO_TICKS(timeout_ms));
}

typedef struct {
    TimerHandle_t h;
    iot_timer_cb_t cb;
    void *arg;
} iot_timer_wrap_t;

static void iot_timer_tramp(TimerHandle_t h)
{
    iot_timer_wrap_t *w = (iot_timer_wrap_t *)pvTimerGetTimerID(h);
    if ((w != NULL) && (w->cb != NULL)) {
        w->cb((iot_timer_t)w, w->arg);
    }
}

iot_err_t iot_timer_create(const char *name, uint32_t period_ms, bool periodic, iot_timer_cb_t cb,
                           void *arg, iot_timer_t *out)
{
    if ((cb == NULL) || (out == NULL) || (period_ms == 0U)) {
        return IOT_ERR_INVALID_ARG;
    }
    iot_timer_wrap_t *w = (iot_timer_wrap_t *)calloc(1, sizeof(*w));
    if (w == NULL) {
        return IOT_ERR_NO_MEM;
    }
    w->cb = cb;
    w->arg = arg;
    w->h = xTimerCreate(name != NULL ? name : "tmr", pdMS_TO_TICKS(period_ms),
                        periodic ? pdTRUE : pdFALSE, w, iot_timer_tramp);
    if (w->h == NULL) {
        free(w);
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_timer_t)w;
    return IOT_OK;
}

iot_err_t iot_timer_start(iot_timer_t t)
{
    iot_timer_wrap_t *w = (iot_timer_wrap_t *)t;
    if ((w == NULL) || (w->h == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    return (xTimerStart(w->h, 0) == pdPASS) ? IOT_OK : IOT_ERR_FAIL;
}

iot_err_t iot_timer_stop(iot_timer_t t)
{
    iot_timer_wrap_t *w = (iot_timer_wrap_t *)t;
    if ((w == NULL) || (w->h == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    return (xTimerStop(w->h, 0) == pdPASS) ? IOT_OK : IOT_ERR_FAIL;
}

void iot_timer_delete(iot_timer_t t)
{
    iot_timer_wrap_t *w = (iot_timer_wrap_t *)t;
    if (w == NULL) {
        return;
    }
    if (w->h != NULL) {
        (void)xTimerDelete(w->h, portMAX_DELAY);
    }
    free(w);
}

void iot_enter_critical(void)
{
    taskENTER_CRITICAL();
}

void iot_exit_critical(void)
{
    taskEXIT_CRITICAL();
}

uint32_t iot_millis(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

#else /* IOT_HOST_SIM */

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#endif

iot_err_t iot_rtos_init(void)
{
    return IOT_OK;
}

#ifdef _WIN32
iot_err_t iot_task_create(const char *name, iot_task_fn_t fn, void *arg, uint32_t stack_bytes,
                          iot_task_prio_t prio, iot_task_t *out)
{
    (void)name;
    (void)prio;
    if ((fn == NULL) || (out == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    HANDLE h = CreateThread(NULL, stack_bytes, (LPTHREAD_START_ROUTINE)fn, arg, 0, NULL);
    if (h == NULL) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_task_t)h;
    return IOT_OK;
}

void iot_task_delay_ms(uint32_t ms)
{
    Sleep(ms == 0U ? 1U : ms);
}

void iot_task_notify_give(iot_task_t task)
{
    (void)task;
}

uint32_t iot_task_notify_take(uint32_t timeout_ms)
{
    Sleep(timeout_ms);
    return 0U;
}

void iot_task_notify_give_from_isr(iot_task_t task, int *hp_woken)
{
    (void)task;
    if (hp_woken != NULL) {
        *hp_woken = 0;
    }
}

iot_err_t iot_mutex_create(iot_mutex_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    HANDLE m = CreateMutex(NULL, FALSE, NULL);
    if (m == NULL) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_mutex_t)m;
    return IOT_OK;
}

void iot_mutex_delete(iot_mutex_t m)
{
    if (m != NULL) {
        (void)CloseHandle((HANDLE)m);
    }
}

iot_err_t iot_mutex_lock(iot_mutex_t m, uint32_t timeout_ms)
{
    if (m == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    DWORD w = WaitForSingleObject((HANDLE)m, timeout_ms);
    return (w == WAIT_OBJECT_0) ? IOT_OK : IOT_ERR_TIMEOUT;
}

void iot_mutex_unlock(iot_mutex_t m)
{
    if (m != NULL) {
        (void)ReleaseMutex((HANDLE)m);
    }
}

iot_err_t iot_sem_binary_create(iot_sem_t *out)
{
    return iot_sem_counting_create(1U, 0U, out);
}

iot_err_t iot_sem_counting_create(uint32_t max_count, uint32_t init, iot_sem_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    HANDLE s = CreateSemaphore(NULL, (LONG)init, (LONG)max_count, NULL);
    if (s == NULL) {
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_sem_t)s;
    return IOT_OK;
}

void iot_sem_delete(iot_sem_t s)
{
    if (s != NULL) {
        (void)CloseHandle((HANDLE)s);
    }
}

iot_err_t iot_sem_take(iot_sem_t s, uint32_t timeout_ms)
{
    if (s == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    DWORD w = WaitForSingleObject((HANDLE)s, timeout_ms);
    return (w == WAIT_OBJECT_0) ? IOT_OK : IOT_ERR_TIMEOUT;
}

void iot_sem_give(iot_sem_t s)
{
    if (s != NULL) {
        (void)ReleaseSemaphore((HANDLE)s, 1, NULL);
    }
}

void iot_sem_give_from_isr(iot_sem_t s, int *hp_woken)
{
    iot_sem_give(s);
    if (hp_woken != NULL) {
        *hp_woken = 0;
    }
}

typedef struct {
    HANDLE mtx;
    HANDLE space;
    HANDLE items;
    uint8_t *buf;
    uint32_t length;
    uint32_t item_size;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} iot_host_queue_t;

iot_err_t iot_queue_create(uint32_t length, uint32_t item_size, iot_queue_t *out)
{
    if ((out == NULL) || (length == 0U) || (item_size == 0U)) {
        return IOT_ERR_INVALID_ARG;
    }
    iot_host_queue_t *q = (iot_host_queue_t *)calloc(1, sizeof(*q));
    if (q == NULL) {
        return IOT_ERR_NO_MEM;
    }
    q->buf = (uint8_t *)calloc(length, item_size);
    q->mtx = CreateMutex(NULL, FALSE, NULL);
    q->space = CreateSemaphore(NULL, (LONG)length, (LONG)length, NULL);
    q->items = CreateSemaphore(NULL, 0, (LONG)length, NULL);
    q->length = length;
    q->item_size = item_size;
    if ((q->buf == NULL) || (q->mtx == NULL) || (q->space == NULL) || (q->items == NULL)) {
        iot_queue_delete((iot_queue_t)q);
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_queue_t)q;
    return IOT_OK;
}

void iot_queue_delete(iot_queue_t qh)
{
    iot_host_queue_t *q = (iot_host_queue_t *)qh;
    if (q == NULL) {
        return;
    }
    if (q->mtx != NULL) {
        (void)CloseHandle(q->mtx);
    }
    if (q->space != NULL) {
        (void)CloseHandle(q->space);
    }
    if (q->items != NULL) {
        (void)CloseHandle(q->items);
    }
    free(q->buf);
    free(q);
}

static iot_err_t host_q_send(iot_host_queue_t *q, const void *item, uint32_t timeout_ms)
{
    if (WaitForSingleObject(q->space, timeout_ms) != WAIT_OBJECT_0) {
        return IOT_ERR_QUEUE_FULL;
    }
    (void)WaitForSingleObject(q->mtx, INFINITE);
    (void)memcpy(q->buf + (q->head * q->item_size), item, q->item_size);
    q->head = (q->head + 1U) % q->length;
    q->count++;
    (void)ReleaseMutex(q->mtx);
    (void)ReleaseSemaphore(q->items, 1, NULL);
    return IOT_OK;
}

iot_err_t iot_queue_send(iot_queue_t q, const void *item, uint32_t timeout_ms)
{
    if ((q == NULL) || (item == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    return host_q_send((iot_host_queue_t *)q, item, timeout_ms);
}

iot_err_t iot_queue_send_from_isr(iot_queue_t q, const void *item, int *hp_woken)
{
    if (hp_woken != NULL) {
        *hp_woken = 0;
    }
    return iot_queue_send(q, item, 0U);
}

iot_err_t iot_queue_recv(iot_queue_t qh, void *item, uint32_t timeout_ms)
{
    iot_host_queue_t *q = (iot_host_queue_t *)qh;
    if ((q == NULL) || (item == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    if (WaitForSingleObject(q->items, timeout_ms) != WAIT_OBJECT_0) {
        return IOT_ERR_TIMEOUT;
    }
    (void)WaitForSingleObject(q->mtx, INFINITE);
    (void)memcpy(item, q->buf + (q->tail * q->item_size), q->item_size);
    q->tail = (q->tail + 1U) % q->length;
    q->count--;
    (void)ReleaseMutex(q->mtx);
    (void)ReleaseSemaphore(q->space, 1, NULL);
    return IOT_OK;
}

uint32_t iot_queue_waiting(iot_queue_t qh)
{
    iot_host_queue_t *q = (iot_host_queue_t *)qh;
    if (q == NULL) {
        return 0U;
    }
    return q->count;
}

typedef struct {
    HANDLE mtx;
    uint32_t bits;
} iot_host_event_t;

iot_err_t iot_event_create(iot_event_t *out)
{
    iot_host_event_t *e = (iot_host_event_t *)calloc(1, sizeof(*e));
    if ((out == NULL) || (e == NULL)) {
        free(e);
        return IOT_ERR_NO_MEM;
    }
    e->mtx = CreateMutex(NULL, FALSE, NULL);
    *out = (iot_event_t)e;
    return IOT_OK;
}

void iot_event_delete(iot_event_t eh)
{
    iot_host_event_t *e = (iot_host_event_t *)eh;
    if (e == NULL) {
        return;
    }
    if (e->mtx != NULL) {
        (void)CloseHandle(e->mtx);
    }
    free(e);
}

void iot_event_set(iot_event_t eh, uint32_t bits)
{
    iot_host_event_t *e = (iot_host_event_t *)eh;
    if (e == NULL) {
        return;
    }
    (void)WaitForSingleObject(e->mtx, INFINITE);
    e->bits |= bits;
    (void)ReleaseMutex(e->mtx);
}

void iot_event_clear(iot_event_t eh, uint32_t bits)
{
    iot_host_event_t *e = (iot_host_event_t *)eh;
    if (e == NULL) {
        return;
    }
    (void)WaitForSingleObject(e->mtx, INFINITE);
    e->bits &= ~bits;
    (void)ReleaseMutex(e->mtx);
}

uint32_t iot_event_wait(iot_event_t eh, uint32_t bits, bool clear, bool wait_all, uint32_t timeout_ms)
{
    iot_host_event_t *e = (iot_host_event_t *)eh;
    uint32_t elapsed = 0U;
    if (e == NULL) {
        return 0U;
    }
    while (elapsed <= timeout_ms) {
        (void)WaitForSingleObject(e->mtx, INFINITE);
        uint32_t cur = e->bits;
        bool ok = wait_all ? ((cur & bits) == bits) : ((cur & bits) != 0U);
        if (ok) {
            if (clear) {
                e->bits &= ~bits;
            }
            (void)ReleaseMutex(e->mtx);
            return cur;
        }
        (void)ReleaseMutex(e->mtx);
        Sleep(5);
        elapsed += 5U;
    }
    return 0U;
}

iot_err_t iot_timer_create(const char *name, uint32_t period_ms, bool periodic, iot_timer_cb_t cb,
                           void *arg, iot_timer_t *out)
{
    (void)name;
    (void)period_ms;
    (void)periodic;
    (void)cb;
    (void)arg;
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    *out = (iot_timer_t)(uintptr_t)1;
    return IOT_OK;
}

iot_err_t iot_timer_start(iot_timer_t t)
{
    (void)t;
    return IOT_OK;
}

iot_err_t iot_timer_stop(iot_timer_t t)
{
    (void)t;
    return IOT_OK;
}

void iot_timer_delete(iot_timer_t t)
{
    (void)t;
}

void iot_enter_critical(void)
{
}

void iot_exit_critical(void)
{
}

uint32_t iot_millis(void)
{
    return (uint32_t)GetTickCount();
}
#else
/* POSIX host — used inside Linux Docker simulation image. */
iot_err_t iot_task_create(const char *name, iot_task_fn_t fn, void *arg, uint32_t stack_bytes,
                          iot_task_prio_t prio, iot_task_t *out)
{
    (void)name;
    (void)stack_bytes;
    (void)prio;
    pthread_t *th = (pthread_t *)calloc(1, sizeof(*th));
    if ((fn == NULL) || (out == NULL) || (th == NULL)) {
        free(th);
        return IOT_ERR_INVALID_ARG;
    }
    if (pthread_create(th, NULL, (void *(*)(void *))fn, arg) != 0) {
        free(th);
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_task_t)th;
    return IOT_OK;
}

void iot_task_delay_ms(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000U);
    ts.tv_nsec = (long)((ms % 1000U) * 1000000UL);
    (void)nanosleep(&ts, NULL);
}

void iot_task_notify_give(iot_task_t task)
{
    (void)task;
}

uint32_t iot_task_notify_take(uint32_t timeout_ms)
{
    iot_task_delay_ms(timeout_ms);
    return 0U;
}

void iot_task_notify_give_from_isr(iot_task_t task, int *hp_woken)
{
    (void)task;
    if (hp_woken != NULL) {
        *hp_woken = 0;
    }
}

iot_err_t iot_mutex_create(iot_mutex_t *out)
{
    pthread_mutex_t *m = (pthread_mutex_t *)calloc(1, sizeof(*m));
    if ((out == NULL) || (m == NULL)) {
        free(m);
        return IOT_ERR_NO_MEM;
    }
    (void)pthread_mutex_init(m, NULL);
    *out = (iot_mutex_t)m;
    return IOT_OK;
}

void iot_mutex_delete(iot_mutex_t m)
{
    if (m != NULL) {
        (void)pthread_mutex_destroy((pthread_mutex_t *)m);
        free(m);
    }
}

iot_err_t iot_mutex_lock(iot_mutex_t m, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (m == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    return (pthread_mutex_lock((pthread_mutex_t *)m) == 0) ? IOT_OK : IOT_ERR_FAIL;
}

void iot_mutex_unlock(iot_mutex_t m)
{
    if (m != NULL) {
        (void)pthread_mutex_unlock((pthread_mutex_t *)m);
    }
}

iot_err_t iot_sem_binary_create(iot_sem_t *out)
{
    return iot_sem_counting_create(1U, 0U, out);
}

iot_err_t iot_sem_counting_create(uint32_t max_count, uint32_t init, iot_sem_t *out)
{
    (void)max_count;
    sem_t *s = (sem_t *)calloc(1, sizeof(*s));
    if ((out == NULL) || (s == NULL)) {
        free(s);
        return IOT_ERR_NO_MEM;
    }
    if (sem_init(s, 0, init) != 0) {
        free(s);
        return IOT_ERR_FAIL;
    }
    *out = (iot_sem_t)s;
    return IOT_OK;
}

void iot_sem_delete(iot_sem_t s)
{
    if (s != NULL) {
        (void)sem_destroy((sem_t *)s);
        free(s);
    }
}

iot_err_t iot_sem_take(iot_sem_t s, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (s == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    return (sem_wait((sem_t *)s) == 0) ? IOT_OK : IOT_ERR_TIMEOUT;
}

void iot_sem_give(iot_sem_t s)
{
    if (s != NULL) {
        (void)sem_post((sem_t *)s);
    }
}

void iot_sem_give_from_isr(iot_sem_t s, int *hp_woken)
{
    iot_sem_give(s);
    if (hp_woken != NULL) {
        *hp_woken = 0;
    }
}

iot_err_t iot_queue_create(uint32_t length, uint32_t item_size, iot_queue_t *out)
{
    (void)length;
    (void)item_size;
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    *out = (iot_queue_t)(uintptr_t)1;
    return IOT_OK;
}

void iot_queue_delete(iot_queue_t q)
{
    (void)q;
}

iot_err_t iot_queue_send(iot_queue_t q, const void *item, uint32_t timeout_ms)
{
    (void)q;
    (void)item;
    (void)timeout_ms;
    return IOT_OK;
}

iot_err_t iot_queue_send_from_isr(iot_queue_t q, const void *item, int *hp_woken)
{
    if (hp_woken != NULL) {
        *hp_woken = 0;
    }
    return iot_queue_send(q, item, 0U);
}

iot_err_t iot_queue_recv(iot_queue_t q, void *item, uint32_t timeout_ms)
{
    (void)q;
    (void)item;
    iot_task_delay_ms(timeout_ms);
    return IOT_ERR_TIMEOUT;
}

uint32_t iot_queue_waiting(iot_queue_t q)
{
    (void)q;
    return 0U;
}

iot_err_t iot_event_create(iot_event_t *out)
{
    uint32_t *bits = (uint32_t *)calloc(1, sizeof(uint32_t));
    if ((out == NULL) || (bits == NULL)) {
        free(bits);
        return IOT_ERR_NO_MEM;
    }
    *out = (iot_event_t)bits;
    return IOT_OK;
}

void iot_event_delete(iot_event_t e)
{
    free(e);
}

void iot_event_set(iot_event_t e, uint32_t bits)
{
    if (e != NULL) {
        *(uint32_t *)e |= bits;
    }
}

void iot_event_clear(iot_event_t e, uint32_t bits)
{
    if (e != NULL) {
        *(uint32_t *)e &= ~bits;
    }
}

uint32_t iot_event_wait(iot_event_t e, uint32_t bits, bool clear, bool wait_all, uint32_t timeout_ms)
{
    (void)wait_all;
    (void)timeout_ms;
    if (e == NULL) {
        return 0U;
    }
    uint32_t cur = *(uint32_t *)e;
    if (clear) {
        *(uint32_t *)e &= ~bits;
    }
    return cur;
}

iot_err_t iot_timer_create(const char *name, uint32_t period_ms, bool periodic, iot_timer_cb_t cb,
                           void *arg, iot_timer_t *out)
{
    (void)name;
    (void)period_ms;
    (void)periodic;
    (void)cb;
    (void)arg;
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    *out = (iot_timer_t)(uintptr_t)1;
    return IOT_OK;
}

iot_err_t iot_timer_start(iot_timer_t t)
{
    (void)t;
    return IOT_OK;
}

iot_err_t iot_timer_stop(iot_timer_t t)
{
    (void)t;
    return IOT_OK;
}

void iot_timer_delete(iot_timer_t t)
{
    (void)t;
}

void iot_enter_critical(void)
{
}

void iot_exit_critical(void)
{
}

uint32_t iot_millis(void)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)((ts.tv_sec * 1000L) + (ts.tv_nsec / 1000000L));
}
#endif /* _WIN32 */
#endif /* ESP_PLATFORM */
