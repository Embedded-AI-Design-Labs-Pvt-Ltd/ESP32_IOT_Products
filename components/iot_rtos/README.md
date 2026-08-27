# iot_rtos

FreeRTOS task architecture and OSAL.

## Tasks

| Task | Priority | Stack | Sync |
| --- | --- | --- | --- |
| sensor | 12 | 4096 | takes `sensor_isr_sem` (ISR or 200 ms poll) |
| proc | 7 | 4096 | receives `sample_q` |
| cmd | 11 | 4096 | receives `command_q` |
| diag | 3 | 3072 | 5 s period |
| wdt | 14 | 2048 | pets task WDT every 2 s |

Wi-Fi, BLE, MQTT, OTA, storage, and application control tasks are started by their service modules and recorded in `iot_task_ctx_t`.

## ISR rule

`iot_sem_give_from_isr(sensor_isr_sem)` only. No logs, no malloc, no driver init.

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
