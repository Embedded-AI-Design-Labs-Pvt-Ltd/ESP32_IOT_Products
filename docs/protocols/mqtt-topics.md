# MQTT topic specification

| Topic | Dir | QoS | Payload |
| --- | --- | --- | --- |
| device/{id}/telemetry | up | 1 | JSON profile measurements |
| device/{id}/status | up | 1 | online/offline |
| device/{id}/command | down | 1 | JSON command |
| device/{id}/config | down | 1 | provisioning/config JSON |
| device/{id}/diagnostics | up | 0 | `iot_diag_snapshot_json` |
| device/{id}/ota | down | 1 | `{"ota_url":"https://..."}` |
