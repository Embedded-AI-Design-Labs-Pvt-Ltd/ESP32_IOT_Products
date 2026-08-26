/**
 * @file iot_exti.h
 * @brief External GPIO interrupt HAL. ISR only gives a semaphore / notifies a task.
 */
#ifndef IOT_EXTI_H
#define IOT_EXTI_H

#include "iot_err.h"
#include "iot_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_EXTI_POSEDGE = 0,
    IOT_EXTI_NEGEDGE,
    IOT_EXTI_ANYEDGE
} iot_exti_edge_t;

iot_err_t iot_exti_init(int pin, iot_exti_edge_t edge, iot_sem_t isr_sem);
iot_err_t iot_exti_deinit(int pin);
uint32_t iot_exti_count(int pin);

#ifdef __cplusplus
}
#endif

#endif /* IOT_EXTI_H */
