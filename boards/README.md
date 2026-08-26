# Board / MCU adaptation

Primary target is **ESP32-C3 + ESP-IDF**. Application code uses `iot_*` HAL only, so other MCUs can replace `iot_drivers` + `iot_rtos`.

| Board | What to replace | Notes |
| --- | --- | --- |
| ESP32-C3 DevKit | nothing | This repo |
| Arduino-ESP32 | still ESP32; prefer ESP-IDF components or wrap Arduino `Wire`/`SPI` behind the same headers | Not a second architecture |
| STM32 (Cube/HAL) | `iot_rtos` → CMSIS-RTOS/FreeRTOS port; drivers → HAL_I2C/SPI/UART | No Wi-Fi/BLE unless companion IC (e.g. ESP-Hosted, NINA) |
| Raspberry Pi | `iot_rtos` POSIX already; drivers → Linux `/dev/i2c-1`, `spidev`, pigpio | Not real-time; use for gateways |

Do not call Arduino `digitalWrite` or STM32 `HAL_GPIO_WritePin` from product profiles.
