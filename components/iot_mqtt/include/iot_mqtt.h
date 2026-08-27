/**
 * @file iot_mqtt.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Generic MQTT service for AWS IoT, Azure IoT Hub, and generic MQTTS.
 */
#ifndef IOT_MQTT_H
#define IOT_MQTT_H

#include "iot_err.h"
#include "iot_types.h"
#include "iot_config.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_MQTT_DOWN = 0,
    IOT_MQTT_CONNECTING,
    IOT_MQTT_UP,
    IOT_MQTT_ERROR
} iot_mqtt_state_t;

iot_err_t iot_mqtt_init(void);
iot_err_t iot_mqtt_start(const iot_runtime_cfg_t *cfg);
iot_err_t iot_mqtt_stop(void);
iot_err_t iot_mqtt_publish_telemetry(const iot_telemetry_msg_t *msg);
iot_mqtt_state_t iot_mqtt_state(void);
void iot_mqtt_build_topic(char *out, size_t out_len, const char *device_id, const char *suffix);

#ifdef __cplusplus
}
#endif

#endif /* IOT_MQTT_H */
