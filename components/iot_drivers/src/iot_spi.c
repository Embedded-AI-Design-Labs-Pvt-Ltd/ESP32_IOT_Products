/**
 * @file iot_spi.c
 * @brief SPI master HAL. Used for MCP2515 CAN controller (not native TWAI).
 */
#include "iot_spi.h"
#include "iot_log.h"
#include "iot_rtos.h"

#ifdef ESP_PLATFORM
#include "driver/spi_master.h"
static spi_device_handle_t s_dev;
#endif

static const char *TAG = "iot_spi";
static iot_mutex_t s_lock;
static bool s_init;

iot_err_t iot_spi_init(const iot_spi_cfg_t *cfg)
{
    if ((cfg == NULL) || s_init) {
        return s_init ? IOT_ERR_ALREADY_INIT : IOT_ERR_INVALID_ARG;
    }
    iot_err_t e = iot_mutex_create(&s_lock);
    if (IOT_FAIL(e)) {
        return e;
    }
#ifdef ESP_PLATFORM
    spi_bus_config_t bus = {
        .mosi_io_num = cfg->mosi,
        .miso_io_num = cfg->miso,
        .sclk_io_num = cfg->sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64,
    };
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return iot_err_from_esp(err);
    }
    spi_device_interface_config_t dev = {
        .clock_speed_hz = (int)(cfg->hz != 0U ? cfg->hz : 1000000U),
        .mode = 0,
        .spics_io_num = cfg->cs,
        .queue_size = 4,
    };
    err = spi_bus_add_device(SPI2_HOST, &dev, &s_dev);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
#else
    (void)cfg;
#endif
    s_init = true;
    IOT_LOGI(TAG, "spi ready (MCP2515 CS must be exclusive)");
    return IOT_OK;
}

iot_err_t iot_spi_deinit(void)
{
    if (!s_init) {
        return IOT_ERR_NOT_INIT;
    }
#ifdef ESP_PLATFORM
    if (s_dev != NULL) {
        (void)spi_bus_remove_device(s_dev);
        s_dev = NULL;
    }
    (void)spi_bus_free(SPI2_HOST);
#endif
    s_init = false;
    return IOT_OK;
}

iot_err_t iot_spi_transfer(const uint8_t *tx, uint8_t *rx, size_t len, uint32_t timeout_ms)
{
    if (!s_init || (tx == NULL) || (len == 0U)) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)iot_mutex_lock(s_lock, timeout_ms);
#ifdef ESP_PLATFORM
    spi_transaction_t t = {0};
    t.length = len * 8;
    t.tx_buffer = tx;
    t.rx_buffer = rx;
    esp_err_t err = spi_device_transmit(s_dev, &t);
    iot_mutex_unlock(s_lock);
    return iot_err_from_esp(err);
#else
    if (rx != NULL) {
        for (size_t i = 0; i < len; i++) {
            rx[i] = tx[i];
        }
    }
    iot_mutex_unlock(s_lock);
    return IOT_OK;
#endif
}
