# iot_drivers

HAL wrapping ESP-IDF. Application code must include these headers only.

| Driver | ESP-IDF API | Notes |
| --- | --- | --- |
| GPIO | `driver/gpio.h` | Mutex-owned |
| UART | `driver/uart.h` | RS485 = UART1 + DE pin + **external MAX3485** |
| I2C | `driver/i2c.h` | Master, mutex-owned |
| SPI | `driver/spi_master.h` | MCP2515 CAN **external** |
| ADC | `esp_adc/adc_oneshot.h` | ADC1 |
| PWM | `driver/ledc.h` | Low-speed LEDC |
| Timer | `driver/gptimer.h` | ISR callback must GiveFromISR only |
| EXTI | GPIO ISR service | `iot_sem_give_from_isr` |
| WDT | `esp_task_wdt.h` | 10 s panic |

## Failure scenarios

| ID | Failure | Handling |
| --- | --- | --- |
| UART-01 | TX timeout | `IOT_ERR_IO`, DE released |
| I2C-01 | NACK | `IOT_ERR_FAIL` from ESP-IDF |
| SPI-01 | MCP2515 missing | transfer may succeed electrically; protocol CRC fails upper layer |
| ADC-01 | Invalid GPIO | `IOT_ERR_INVALID_ARG` |
| EXTI-01 | ISR flood | counting semaphore / drop at sensor queue |
