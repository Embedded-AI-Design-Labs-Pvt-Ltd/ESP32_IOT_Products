/**
 * @file iot_spi.h
 * @brief SPI master HAL. Chip-select IOT_PIN_SPI_CS_CAN is reserved for MCP2515.
 */
#ifndef IOT_SPI_H
#define IOT_SPI_H

#include <stdint.h>
#include <stddef.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int mosi;
    int miso;
    int sclk;
    int cs;
    uint32_t hz;
} iot_spi_cfg_t;

iot_err_t iot_spi_init(const iot_spi_cfg_t *cfg);
iot_err_t iot_spi_deinit(void);
iot_err_t iot_spi_transfer(const uint8_t *tx, uint8_t *rx, size_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* IOT_SPI_H */
