# iot_profiles

Kconfig selects one profile. Do not fork platform APIs.

| Profile | External hardware |
| --- | --- |
| Energy | Voltage divider, CT/shunt |
| Industrial | TMP117, vibration, CT, 4-20 mA |
| Predictive | IMU or analog vibration |
| Water | Hall flow, tank analog, valve GPIO |
| EV charger | **Telemetry only** — safety on separate controller |
| Automotive | **MCP2515 + transceiver** |
| Asset tracker | BLE; optional GNSS UART |
| Agriculture | Soil ADC, pump PWM |
| Cold-chain | SHT3x/TMP117 |
| Robotics | UART to robot MCU |

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
