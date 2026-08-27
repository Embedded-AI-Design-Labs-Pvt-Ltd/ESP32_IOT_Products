# MQTT topic specification

| Topic | Dir | QoS | Payload |
| --- | --- | --- | --- |
| device/{id}/telemetry | up | 1 | JSON profile measurements |
| device/{id}/status | up | 1 | online/offline |
| device/{id}/command | down | 1 | JSON command |
| device/{id}/config | down | 1 | provisioning/config JSON |
| device/{id}/diagnostics | up | 0 | `iot_diag_snapshot_json` |
| device/{id}/ota | down | 1 | `{"ota_url":"https://..."}` |

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
