# Use cases (C / C++)

Dispatch: `iot_uc_run_json("{\"uc\":\"name\",...}", n, &out)` or `iot::UseCaseEngine::runJson`.

MQTT/BLE command payloads with a `"uc"` key are executed on the command task after `iot_security_validate_command`.

## Platform

| `uc` | Behavior |
| --- | --- |
| `plt.factory_reset` | NVS defaults, unprovisioned |
| `plt.set_sample_ms` | 50–60000 ms sample period |
| `plt.get_diag` | Diagnostic snapshot JSON |
| `plt.provision` | Commissioning JSON → NVS |
| `plt.ota` | HTTPS OTA only |
| `plt.identity` | Device id / fw / profile |
| `plt.set_cloud` | 0 generic, 1 AWS, 2 Azure, 3 GCP MQTT |
| `plt.set_log_level` | 0–5 |
| `plt.export_topics` | MQTT suffix list |
| `plt.wifi_backoff` | 1 s–30 s reconnect policy |
| `plt.cmd_schema` | JSON object/array gate |
| `plt.mark_ota_valid` | Cancel rollback |
| `plt.shutdown` | `IOT_EVT_SHUTDOWN` |

## Smart energy

`energy.measure`, `energy.accum`, `energy.over_power`, `energy.over_voltage`, `energy.under_voltage`, `energy.over_current`, `energy.set_limits`, `energy.calibrate`, `energy.reset_daily`, `energy.pf`, `energy.get_state`, `energy.led_alarm`

## Industrial / predictive

`industrial.sample`, `industrial.state`, `industrial.set_limits`, `industrial.ack`, `industrial.hours`, `industrial.pulse`

`predictive.features`, `predictive.health`, `predictive.trend`, `predictive.reset_peak`

## Water

`water.meter`, `water.leak`, `water.burst`, `water.valve_open`, `water.valve_close`, `water.tamper`, `water.billing_freeze`, `water.set_k`, `water.get_state`

## EV gateway (not a safety controller)

`ev.sample`, `ev.start`, `ev.stop`, `ev.reject_contactor` (**always denied**), `ev.session`, `ev.fault`, `ev.get_state`

## Automotive (MCP2515, not native C3 CAN)

`auto.filter`, `auto.decode`, `auto.reject_tx` (**always denied**), `auto.bus_off`, `auto.dtc`, `auto.get_state`

## Tracker / agriculture / cold-chain / robotics

`tracker.beacon`, `tracker.motion`, `tracker.batt_low`, `tracker.geofence`, `tracker.sleep`

`agri.soil`, `agri.pump_auto`, `agri.pump_on`, `agri.pump_off`, `agri.schedule`, `agri.inhibit`

`cold.sample`, `cold.excursion`, `cold.door`, `cold.ack`, `cold.datalog`

`robot.uart`, `robot.heartbeat`, `robot.reject_motion` (**always denied**), `robot.estop_report`, `robot.battery`

## Example

```c
iot_uc_out_t out;
iot_uc_run_json("{\"uc\":\"water.valve_close\"}", 28, &out);
```

```cpp
iot::UseCaseEngine eng;
iot_uc_out_t out{};
eng.runJson(R"({"uc":"energy.measure","v":230,"i":1.0})", out);
```

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
