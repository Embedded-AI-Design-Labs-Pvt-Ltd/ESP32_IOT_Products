# Build / flash / debug

## Build
```
idf.py set-target esp32c3
idf.py build
```

## Flash
```
idf.py -p COMx flash
```

## Debug
- UART0 115200: IDF log tags `app_main`, `iot_*`
- `idf.py monitor`
- Core dump partition `coredump` in `partitions.csv`
- OpenOCD + RISC-V gdb optional (`idf.py gdb`)

## Troubleshooting

| Symptom | Check |
| --- | --- |
| Brownout | 3.3 V current; Wi-Fi TX spikes |
| WDT reset | blocked task; increase work or pet WDT only from wdt task |
| MQTT TLS fail | URI `mqtts://`, time, cert bundle |
| BLE not visible | NimBLE enabled in sdkconfig; name = device id |
| CAN silent | MCP2515 wiring, 8 MHz crystal, 120 Ω termination — **not** native C3 CAN |

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
