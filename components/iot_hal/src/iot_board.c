/**
 * @file iot_board.c
 * @brief Board bring-up: reset reason, identity of native vs external silicon.
 *
 * @syscalls esp_reset_reason, esp_rom_printf (via log)
 * @thread startup / main
 */
#include "iot_board.h"
#include "iot_log.h"

#ifdef ESP_PLATFORM
#include "esp_system.h"
#include "esp_chip_info.h"
#endif

static const char *TAG = "iot_board";
static const iot_board_info_t s_info = {
    .name = "ESP32-C3-DevKitM-1 compatible",
    .mcu = "ESP32-C3 (RISC-V, 160 MHz, Wi-Fi + BLE 5)",
    .notes = "CAN=MCP2515+transceiver; RS485=UART1+MAX3485; analog via ADC1.",
    .native_can = false,
    .native_ethernet = false,
};

iot_err_t iot_board_init(void)
{
    IOT_LOGI(TAG, "%s", s_info.mcu);
    IOT_LOGI(TAG, "native_can=%d native_eth=%d", (int)s_info.native_can, (int)s_info.native_ethernet);
#ifdef ESP_PLATFORM
    IOT_LOGI(TAG, "reset reason=%s", iot_board_reset_reason());
#endif
    return IOT_OK;
}

const iot_board_info_t *iot_board_info(void)
{
    return &s_info;
}

const char *iot_board_reset_reason(void)
{
#ifdef ESP_PLATFORM
    switch (esp_reset_reason()) {
    case ESP_RST_POWERON:
        return "POWERON";
    case ESP_RST_SW:
        return "SW";
    case ESP_RST_PANIC:
        return "PANIC";
    case ESP_RST_INT_WDT:
        return "INT_WDT";
    case ESP_RST_TASK_WDT:
        return "TASK_WDT";
    case ESP_RST_WDT:
        return "WDT";
    case ESP_RST_DEEPSLEEP:
        return "DEEPSLEEP";
    case ESP_RST_BROWNOUT:
        return "BROWNOUT";
    case ESP_RST_SDIO:
        return "SDIO";
    default:
        return "UNKNOWN";
    }
#else
    return "HOST_SIM";
#endif
}
