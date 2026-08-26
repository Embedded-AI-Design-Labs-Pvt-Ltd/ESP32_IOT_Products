# iot_ble

NimBLE GATT service `IOT1` characteristics:

| UUID | Name | Properties |
| --- | --- | --- |
| 0xFF01 | Configuration | R/W (provision JSON) |
| 0xFF02 | Sensor | R/N |
| 0xFF03 | Control | W |
| 0xFF04 | Diagnostics | R |
| 0xFF05 | Firmware version | R |

Callbacks only enqueue `command_q` (timeout 0).
