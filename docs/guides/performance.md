# Performance budgets (ESP32-C3, 160 MHz, ~400 KB SRAM)

| Resource | Budget | Notes |
| --- | --- | --- |
| Task stacks (sum) | < 40 KB | listed in iot_tasks |
| MQTT JSON | ≤ 768 B | `IOT_JSON_TELEMETRY_LEN` |
| sample_q / telemetry_q | 16 × telemetry | drop + log on full |
| command_q | 8 | BLE/MQTT timeout 0 never blocks callbacks |
| Interrupt | ISR < 20 µs goal | GiveFromISR only |
| Sensor period | default 200 ms wait + profile | Kconfig `sample_ms` |
| Wi-Fi reconnect | 1 s … 30 s backoff | event path |
| Heap after boot | log via diag snapshot | fail DIAG_HEAP_LOW if added |

## Known risks

- Priority inversion: resource locks are **mutexes** (priority inheritance on FreeRTOS), not binary semaphores.
- Copies: telemetry structs are copied by value through two queues (bounded, explicit).
- BLE notify allocates `os_mbuf` — skip if disconnected.
- Wi-Fi event handler must not long-block (current reconnect uses a short delay cap; prefer `esp_timer` in a follow-up).

## Optimization recommendations

1. Enable compiler `-Os` for production.
2. Reduce telemetry JSON with CBOR if cellular/BLE-only products appear.
3. Pin MQTT task stack after watermark measurement (`uxTaskGetStackHighWaterMark`).
4. Do not increase `configLOG_DEFAULT_LEVEL` in production.
