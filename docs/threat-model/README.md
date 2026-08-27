# Threat model (Agent 8)

| Threat | Attack | Mitigation | Residual |
| --- | --- | --- | --- |
| Rogue device | Clone MAC, fake telemetry | Device certs / TPM-less: per-device key in protected NVS; cloud registry | Physical dump of flash if encryption off |
| MITM | Fake AP / broker | TLS 1.2 + cert bundle; MQTTS only in production URI | Clock/TLS intercept if bundle outdated |
| Unauthorized firmware | Modified image | Secure Boot + signed OTA (enable in production overlay); HTTPS OTA; rollback | Keys must be generated offline |
| Credential theft | Repo secrets, UART dump | No secrets in source; NVS; disable UART download in production | Physical access |
| Replay | Repeat MQTT start/stop | Cloud-side nonce/timestamp (future); local schema checks | Limited without timestamps |
| Malicious MQTT commands | `can_tx`, `contactor` | Profile deny lists; `iot_security_validate_command` | Logic bugs |
| Physical access | Debug, strapping | Flash encryption, disable JTAG in production sdkconfig | Lab units remain open |

## NVS protection strategy

- Default: unencrypted NVS for development.
- Production: enable `CONFIG_NVS_ENCRYPTION` and flash the `nvs_key` partition generated offline.
- Never log Wi-Fi or MQTT passwords.

## Secure boot concepts

ESP32-C3 Secure Boot v2 and flash encryption are **documented and default-off**. Enabling them requires signing keys not stored in this repository. See Espressif Secure Boot docs. Use a production `sdkconfig` overlay in CI secrets, not git.

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
