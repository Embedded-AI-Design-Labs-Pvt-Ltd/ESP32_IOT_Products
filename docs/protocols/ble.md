# BLE protocol

Device name = NVS device id.

Generic IoT service UUID 128-bit `IOT1` (see `iot_ble.c`).

Provisioning write example:

```json
{"ssid":"plant-wifi","pass":"********","mqtt":"mqtts://xxx:8883"}
```

Passwords must not be logged.
