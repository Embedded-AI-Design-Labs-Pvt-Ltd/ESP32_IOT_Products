# Cursor Agent Team — ESP32-C3 IoT Platform

Project-level subagents live in `.cursor/agents/`. Invoke them in order. Do not run all agents randomly.

## Agents

| Agent | Cursor name | Responsibility |
| --- | --- | --- |
| Agent 1 | `system-architect` | System/software/task/data-flow/security/OTA architecture |
| Agent 2 | `esp-idf-board` | ESP-IDF project, board, partitions, startup, build |
| Agent 3 | `freertos` | Tasks and synchronization |
| Agent 4 | `driver` | UART/I2C/SPI/GPIO/ADC/PWM/timer/ISR/WDT HAL |
| Agent 5 | `ble` | BLE GATT and BLE provisioning |
| Agent 6 | `wifi` | Wi-Fi/network manager |
| Agent 7 | `mqtt-cloud` | MQTT and AWS/Azure/GCP broker profiles |
| Agent 8 | `security` | TLS, identity, OTA, threat model |
| Agent 9 | `diagnostics` | Faults, logging, health snapshots |
| Agent 10 | `iot-application` | Ten product application profiles |
| Agent 11 | `testing` | Unit, integration, HIL, fault, stress |
| Agent 12 | `performance` | CPU/RAM/latency/power |
| Agent 13 | `documentation` | Architecture, API, HTML docs, GUI help |
| Agent 14 | `devops` | Docker, Jenkins, reproducible release |
| Final | `principal-integration` | Architecture/code review and readiness |

## Recommended execution order

1. Foundation: `system-architect` → `esp-idf-board` → `driver` → `freertos`
2. Connectivity: `ble` → `wifi` → `mqtt-cloud` → `security`
3. Platform services: `diagnostics` (storage/config included)
4. Products: `iot-application` (one profile at a time)
5. Quality: `testing` → `performance`
6. Release: `documentation` → `devops` → `principal-integration`

## Invoke

```
Use the system-architect subagent to review layered interfaces.
Use the principal-integration subagent to review ISR and queue ownership.
```

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
