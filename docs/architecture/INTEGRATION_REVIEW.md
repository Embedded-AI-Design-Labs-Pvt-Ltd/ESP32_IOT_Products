# Architecture and code review (Principal Embedded Software Engineer)

Review date: 2026-08-26  
Scope: greenfield repository `ESP32_IOT_Products` (no prior firmware to preserve).

## Architecture consistency

Layering is encoded in components and `.cursor/rules`. Profiles include HAL + services, not `driver/gpio.h`. CAN is documented as external MCP2515.

## Defects found and disposition

| ID | Finding | Severity | Disposition |
| --- | --- | --- | --- |
| R1 | Wi-Fi reconnect used a delay inside the IDF event handler | Medium | Capped; remaining risk: prefer `esp_timer` reconnect task |
| R2 | Host POSIX queue is a stub (Linux Docker sim of queues) | Medium | Win32 host queue is real; Linux pthread path simplified |
| R3 | BLE Just Works pairing | Medium | Documented; raise IO cap for production |
| R4 | Energy ADC pin vs SPI SCLK possible conflict | Low | Board overlay required; noted in `iot_board.h` |
| R5 | `iot_config_init` returns `ALREADY_INIT` on second call | Low | Call once from `app_main` |
| R6 | Telemetry copied by value (large struct) | Low | Bounded; acceptable for C3 |
| R7 | Secure Boot default off | Info | Required for production; keys not in git |

## Defects fixed during integration

- BLE sensor cache visible on host compile.
- MQTT includes cert bundle header.
- Driver CMake does not depend on IDF 5.3-only `esp_driver_gpio`.
- EXTI ISR includes FreeRTOS portmacro for `portYIELD_FROM_ISR`.
- Command validation rejects empty/non-JSON actuator payloads.
- EV/automotive profiles deny safety/CAN inject commands.

## Remaining risks

- First IDF build not executed in this environment (no IDF on the authoring machine). Use Docker `Dockerfile.idf`.
- NimBLE UUID/GATT registration must be verified on hardware.
- Wi-Fi event-loop blocking must be re-tested under AP flap.
- Host Linux queue stub: run Win32 tests or complete POSIX queue before relying on Linux HIL-less queue tests.

## Test coverage

Host unit tests: error, config, mutex, sem, queue, gpio, security, diag, provision, mqtt topic, energy JSON.

On-target/HIL tests are specified in `docs/guides/test-plan.md` (not executed here).

## Memory / performance

See `docs/guides/performance.md`. Dual-OTA 4 MB layout. Task stacks ~4 KB typical.

## Production-readiness assessment

**Prototype / production-oriented platform: YES for architecture, HAL, task model, MQTT/BLE/OTA/security policy.**  
**Factory production: NOT YET** until Secure Boot + flash encryption + encrypted NVS + HIL soak + signed OTA keys + hardware bring-up of NimBLE/Wi-Fi/MCP2515.

## ISR / queue ownership (traced)

1. Pulse GPIO → `gpio_isr` → `iot_sem_give_from_isr(sensor_isr_sem)` → Sensor task → `sample_q` (module-owned) → Proc → `telemetry_q` → MQTT pump → broker.  
2. Broker → MQTT_EVENT_DATA → `command_q` timeout 0 → Cmd task → validate → profile HAL.

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
