/**
 * @file iot_config.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Compile-time and runtime configuration keys (NVS-backed).
 *
 * Production credentials are never stored in this header. Keys are names only.
 */
#ifndef IOT_CONFIG_H
#define IOT_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "iot_err.h"
#include "iot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_CFG_NS                 "iot"
#define IOT_CFG_KEY_DEVICE_ID      "device_id"
#define IOT_CFG_KEY_WIFI_SSID      "wifi_ssid"
#define IOT_CFG_KEY_WIFI_PASS      "wifi_pass"
#define IOT_CFG_KEY_MQTT_URI       "mqtt_uri"
#define IOT_CFG_KEY_MQTT_USER      "mqtt_user"
#define IOT_CFG_KEY_MQTT_PASS      "mqtt_pass"
#define IOT_CFG_KEY_CLOUD          "cloud_id"
#define IOT_CFG_KEY_SAMPLE_MS      "sample_ms"
#define IOT_CFG_KEY_PROVISIONED    "provisioned"

typedef struct {
    char device_id[IOT_DEVICE_ID_LEN];
    char wifi_ssid[33];
    char wifi_pass[65];
    char mqtt_uri[128];
    char mqtt_user[64];
    char mqtt_pass[64];
    uint32_t sample_period_ms;
    iot_cloud_provider_t cloud;
    bool provisioned;
} iot_runtime_cfg_t;

iot_err_t iot_config_init(void);
iot_err_t iot_config_load(iot_runtime_cfg_t *out);
iot_err_t iot_config_save(const iot_runtime_cfg_t *in);
iot_err_t iot_config_set_defaults(iot_runtime_cfg_t *out);
const iot_runtime_cfg_t *iot_config_get(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_CONFIG_H */
