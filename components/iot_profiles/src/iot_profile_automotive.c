/**
 * @file iot_profile_automotive.c
 * @brief Automotive CAN gateway — MCP2515 frames into auto.* use cases.
 */
#include "iot_profile.h"
#include "iot_spi.h"
#include "iot_board.h"
#include "iot_services.h"
#include "iot_log.h"
#include "iot_uc.h"

#include <string.h>

static const char *TAG = "auto";

iot_err_t iot_profile_automotive_init(void)
{
    iot_spi_cfg_t spi = {
        .mosi = IOT_PIN_SPI_MOSI,
        .miso = IOT_PIN_SPI_MISO,
        .sclk = IOT_PIN_SPI_SCLK,
        .cs = IOT_PIN_SPI_CS_CAN,
        .hz = 1000000,
    };
    (void)iot_spi_init(&spi);
    IOT_LOGW(TAG, "CAN is MCP2515+transceiver — not native on ESP32-C3");
    return IOT_OK;
}

static uint32_t decode_u16_be(const uint8_t *d)
{
    return ((uint32_t)d[0] << 8) | d[1];
}

iot_err_t iot_profile_automotive_sensor(iot_telemetry_msg_t *out)
{
    uint8_t tx[8] = {0x90, 0, 0, 0, 0, 0, 0, 0};
    uint8_t rx[8] = {0};
    iot_uc_out_t uco;
    (void)iot_spi_transfer(tx, rx, sizeof(tx), 20U);
    uint32_t can_id = (uint32_t)rx[1];
    uint32_t speed = decode_u16_be(&rx[2]) % 250U;
    uint32_t rpm = 800U + (decode_u16_be(&rx[4]) % 4000U);
    (void)iot_uc_auto_on_frame(can_id, speed, rpm, rx[6], &uco);
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_automotive_cmd(const iot_command_msg_t *cmd)
{
    if (cmd == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (strstr((const char *)cmd->payload, "can_tx") != NULL) {
        iot_uc_out_t o;
        return iot_uc_run_json("{\"uc\":\"auto.reject_tx\"}", 26, &o);
    }
    return iot_services_handle_command(cmd);
}
