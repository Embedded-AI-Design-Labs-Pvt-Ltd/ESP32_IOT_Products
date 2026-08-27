# iot_core

Thread-safe configuration, error model, and logging used by every layer.

## Public API

- `iot_err_t` / `iot_err_from_esp()` / `iot_err_to_name()`
- `iot_log_*` — not ISR-safe
- `iot_config_init/load/save` — mutex-owned NVS cache

## Failure scenarios

| ID | Scenario | Recovery |
| --- | --- | --- |
| CFG-01 | NVS empty | Defaults loaded, device unprovisioned |
| CFG-02 | NVS corrupt | ESP-IDF erases namespace; defaults restored |
| LOG-01 | Called from ISR | Forbidden; use diagnostic ISR counters |

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
