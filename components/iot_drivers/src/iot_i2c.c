/**
 * @file iot_i2c.c
 * @brief I2C master HAL (legacy driver/i2c.h — ESP-IDF 5.x compatible).
 */
#include "iot_i2c.h"
#include "iot_log.h"
#include "iot_rtos.h"

#include <string.h>

#ifdef ESP_PLATFORM
#include "driver/i2c.h"
#define I2C_PORT I2C_NUM_0
#endif

static const char *TAG = "iot_i2c";
static iot_mutex_t s_lock;
static bool s_init;

iot_err_t iot_i2c_init(const iot_i2c_cfg_t *cfg)
{
    if ((cfg == NULL) || s_init) {
        return s_init ? IOT_ERR_ALREADY_INIT : IOT_ERR_INVALID_ARG;
    }
    iot_err_t e = iot_mutex_create(&s_lock);
    if (IOT_FAIL(e)) {
        return e;
    }
#ifdef ESP_PLATFORM
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = cfg->sda,
        .scl_io_num = cfg->scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = cfg->hz != 0U ? cfg->hz : 100000U,
    };
    esp_err_t err = i2c_param_config(I2C_PORT, &conf);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    err = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
#else
    (void)cfg;
#endif
    s_init = true;
    IOT_LOGI(TAG, "i2c master ready");
    return IOT_OK;
}

iot_err_t iot_i2c_deinit(void)
{
    if (!s_init) {
        return IOT_ERR_NOT_INIT;
    }
#ifdef ESP_PLATFORM
    (void)i2c_driver_delete(I2C_PORT);
#endif
    s_init = false;
    return IOT_OK;
}

iot_err_t iot_i2c_write(uint8_t addr7, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (!s_init || (data == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock, timeout_ms);
#ifdef ESP_PLATFORM
    esp_err_t err = i2c_master_write_to_device(I2C_PORT, addr7, data, len, pdMS_TO_TICKS(timeout_ms));
    iot_mutex_unlock(s_lock);
    return iot_err_from_esp(err);
#else
    (void)addr7;
    (void)len;
    iot_mutex_unlock(s_lock);
    return IOT_OK;
#endif
}

iot_err_t iot_i2c_read(uint8_t addr7, uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (!s_init || (data == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock, timeout_ms);
#ifdef ESP_PLATFORM
    esp_err_t err = i2c_master_read_from_device(I2C_PORT, addr7, data, len, pdMS_TO_TICKS(timeout_ms));
    iot_mutex_unlock(s_lock);
    return iot_err_from_esp(err);
#else
    (void)addr7;
    (void)len;
    (void)memset(data, 0, len);
    iot_mutex_unlock(s_lock);
    return IOT_OK;
#endif
}

iot_err_t iot_i2c_write_read(uint8_t addr7, const uint8_t *wr, size_t wr_len, uint8_t *rd, size_t rd_len,
                             uint32_t timeout_ms)
{
    if (!s_init || (wr == NULL) || (rd == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock, timeout_ms);
#ifdef ESP_PLATFORM
    esp_err_t err = i2c_master_write_read_device(I2C_PORT, addr7, wr, wr_len, rd, rd_len,
                                                 pdMS_TO_TICKS(timeout_ms));
    iot_mutex_unlock(s_lock);
    return iot_err_from_esp(err);
#else
    (void)addr7;
    (void)wr_len;
    (void)memset(rd, 0, rd_len);
    iot_mutex_unlock(s_lock);
    return IOT_OK;
#endif
}
