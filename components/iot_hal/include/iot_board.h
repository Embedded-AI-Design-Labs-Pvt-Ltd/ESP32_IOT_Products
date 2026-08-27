/**
 * @file iot_board.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief ESP32-C3-DevKitM-1 default pin map and external hardware contracts.
 *
 * Native on ESP32-C3: GPIO, UART, I2C, SPI, ADC1, LEDC PWM, GPTimer, Wi-Fi, BLE 5.
 * NOT native: CAN/TWAI, Ethernet MAC, USB-OTG host. Automotive CAN requires MCP2515
 * (SPI) + TJA1050/SN65HVD230 transceiver. RS485 requires UART + MAX3485 DE/RE.
 */
#ifndef IOT_BOARD_H
#define IOT_BOARD_H

#include <stdint.h>
#include <stdbool.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_PIN_LED           8  /**< On-board RGB/LED on many C3 DevKits. */
#define IOT_PIN_BUTTON        9
#define IOT_PIN_UART0_TX      21
#define IOT_PIN_UART0_RX      20
#define IOT_PIN_UART1_TX      4
#define IOT_PIN_UART1_RX      5
#define IOT_PIN_RS485_DE      6  /**< External MAX3485 DE/RE (active high DE). */
#define IOT_PIN_I2C_SDA       2
#define IOT_PIN_I2C_SCL       3
#define IOT_PIN_SPI_MOSI      7
#define IOT_PIN_SPI_MISO      10
#define IOT_PIN_SPI_SCLK      1
#define IOT_PIN_SPI_CS_CAN    0  /**< MCP2515 chip-select — not native CAN. */
#define IOT_PIN_ADC_V         0  /**< ADC1_CH0 — voltage sense (energy/EV). GPIO0. */
#define IOT_PIN_ADC_I         1  /**< ADC1_CH1 — current sense. Conflicts with SCLK if both used; board overlay required. */
#define IOT_PIN_PWM_OUT       18
#define IOT_PIN_EXTI_PULSE    19 /**< Flow/vibration pulse ISR input. */

typedef struct {
    const char *name;
    const char *mcu;
    const char *notes;
    bool native_can;
    bool native_ethernet;
} iot_board_info_t;

iot_err_t iot_board_init(void);
const iot_board_info_t *iot_board_info(void);
const char *iot_board_reset_reason(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_BOARD_H */
