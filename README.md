# ESP32-C3 Commercial IoT Platform

Reusable production-oriented firmware for **ESP32-C3** (ESP-IDF + FreeRTOS) that is adapted into ten product profiles rather than ten separate codebases.

| Layer | Component |
| --- | --- |
| Application | `components/iot_profiles` (10 Kconfig profiles) |
| IoT services | `iot_services`, `iot_provision` |
| Communication | `iot_wifi`, `iot_ble`, `iot_mqtt` |
| Device services | `iot_diag`, `iot_ota`, `iot_security`, `iot_core` (NVS config) |
| Drivers/HAL | `iot_drivers`, `iot_hal` |
| OS | `iot_rtos` (FreeRTOS on target, Win32/POSIX on host) |
| SDK / silicon | ESP-IDF 5.1+ / ESP32-C3 |

ESP32-C3 has **no native CAN**. Automotive profiles require **MCP2515 (SPI) + CAN transceiver**. RS485 requires **UART + MAX3485 DE/RE**.

## Cursor agents

Project subagents are in `.cursor/agents/`. See [AGENTS.md](AGENTS.md) for the 14-agent team and execution order.

## Build firmware (hardware)

```bash
# ESP-IDF 5.1+ on PATH, or Docker:
docker build -f docker/Dockerfile.idf -t esp32c3-iot-idf .
docker run --rm -v ${PWD}:/project -w /project esp32c3-iot-idf bash -lc "idf.py set-target esp32c3 && idf.py build"
idf.py -p COMx flash monitor
```

Select a product in `idf.py menuconfig` → **IoT Core** → Product application profile.

## Simulation (no hardware)

```bash
cmake -S tests/host -B host_build
cmake --build host_build
ctest --test-dir host_build --output-on-failure

python tools/gui/server.py
# http://127.0.0.1:8080  — live telemetry sim, task table, Mermaid flows
```

Docker without hardware:

```bash
docker compose -f docker/docker-compose.yml up simulator mosquitto
```

Hardware IDF builder (optional profile):

```bash
docker compose -f docker/docker-compose.yml --profile hardware up idf-builder
```

## Cloud

MQTT topics (TLS):

- `device/{id}/telemetry`
- `device/{id}/status`
- `device/{id}/command`
- `device/{id}/config`
- `device/{id}/diagnostics`
- `device/{id}/ota`

Provider overlays: [cloud/aws](cloud/aws), [cloud/azure](cloud/azure), [cloud/gcp](cloud/gcp). **Do not commit certificates.**

## Documentation

- Architecture: [docs/architecture](docs/architecture)
- HTML console: `tools/gui/static/index.html`
- Threat model: [docs/threat-model](docs/threat-model)
- Tests: [docs/guides/test-plan.md](docs/guides/test-plan.md)
- Performance: [docs/guides/performance.md](docs/guides/performance.md)

## CI

Jenkins pipeline: [ci/jenkins/Jenkinsfile](ci/jenkins/Jenkinsfile) — host tests, optional cppcheck/clang-format, Docker IDF build, HTML docs, firmware artifacts.

## License

Apache-2.0 (firmware examples). Hardware third-party ICs remain their vendors' IP.
