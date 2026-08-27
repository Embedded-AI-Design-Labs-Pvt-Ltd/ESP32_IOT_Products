/**
 * @file iot_i2c.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief I2C master HAL wrapping ESP-IDF driver/i2c.h.
 */
#ifndef IOT_I2C_H
#define IOT_I2C_H

#include <stdint.h>
#include <stddef.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sda;
    int scl;
    uint32_t hz;
} iot_i2c_cfg_t;

iot_err_t iot_i2c_init(const iot_i2c_cfg_t *cfg);
iot_err_t iot_i2c_deinit(void);
iot_err_t iot_i2c_write(uint8_t addr7, const uint8_t *data, size_t len, uint32_t timeout_ms);
iot_err_t iot_i2c_read(uint8_t addr7, uint8_t *data, size_t len, uint32_t timeout_ms);
iot_err_t iot_i2c_write_read(uint8_t addr7, const uint8_t *wr, size_t wr_len, uint8_t *rd, size_t rd_len,
                             uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* IOT_I2C_H */
