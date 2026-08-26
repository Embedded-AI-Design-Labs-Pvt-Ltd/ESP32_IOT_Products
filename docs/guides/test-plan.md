# Test plan

Every automated test below is implemented in `tests/host/test_runner.c` unless marked HIL.

| Test ID | Module | Precondition | Input | Expected | Pass/fail |
| --- | --- | --- | --- | --- | --- |
| UT-ERR-01 | core | none | IOT_OK | name `IOT_OK` | string match |
| UT-CFG-01 | config | host | init | IOT_OK | rc |
| UT-RTOS-02 | mutex | init | create+lock | IOT_OK | rc |
| UT-RTOS-04 | binary sem | init | give/take | take succeeds | rc |
| UT-RTOS-07 | queue | create | send 7 | recv 7 | value |
| UT-GPIO-03 | gpio | init out | write 1 | read 1 | sim shadow |
| UT-SEC-02 | security | init | empty cmd | IOT_ERR_DENIED | rc |
| UT-DIAG-02 | diag | event wifi fail | snapshot | wifi_fails>=1 | counter |
| UT-PROV-02 | provision | JSON ssid | apply | provisioned | flag |
| UT-MQTT-01 | mqtt | none | build topic | device/c3-1/telemetry | string |
| UT-PROF-02 | energy | gpio+adc sim | sensor hook | payload_len>0 | JSON |

## Integration (on-target / HIL)

| Test ID | Precondition | Expected |
| --- | --- | --- |
| IT-WIFI-01 | valid SSID in NVS | WIFI_IP_READY < 30 s |
| IT-MQTT-01 | broker + certs | publish telemetry QoS1 |
| IT-BLE-01 | phone nRF Connect | config write provisions NVS |
| IT-OTA-01 | https URL signed image | reboot into new slot; mark valid |
| IT-WDT-01 | starve wdt task | TASK_WDT reset; DIAG_WATCHDOG_RESET |
| FT-I2C-01 | disconnect SDA | IOT_ERR_* and DIAG_I2C_ERROR |
| ST-Q-01 | 1 kHz fake ISR 60 s | no sample_q overflow storm / bounded drops |
| LT-24H-01 | soak 24 h | heap min stable, no WDT |

## Fault injection

- MQTT broker down → reconnect, `DIAG_MQTT_DISCONNECT`
- Invalid OTA URL (`http://`) → `IOT_ERR_DENIED`
- CAN cloud `can_tx` → `IOT_ERR_DENIED`
- EV `contactor` command → `IOT_ERR_DENIED`
