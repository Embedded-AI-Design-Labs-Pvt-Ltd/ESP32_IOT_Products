/**
 * @file iot_types.h
 * @brief Common scalar types, device identity, and telemetry envelopes.
 */
#ifndef IOT_TYPES_H
#define IOT_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "iot_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_DEVICE_ID_LEN      32U
#define IOT_FW_VERSION_LEN     16U
#define IOT_TOPIC_LEN          96U
#define IOT_JSON_SMALL_LEN     256U
#define IOT_JSON_TELEMETRY_LEN 768U

typedef enum {
    IOT_PROFILE_ENERGY = 0,
    IOT_PROFILE_INDUSTRIAL,
    IOT_PROFILE_PREDICTIVE,
    IOT_PROFILE_WATER,
    IOT_PROFILE_EV_CHARGER,
    IOT_PROFILE_AUTOMOTIVE,
    IOT_PROFILE_ASSET_TRACKER,
    IOT_PROFILE_AGRICULTURE,
    IOT_PROFILE_COLDCHAIN,
    IOT_PROFILE_ROBOTICS,
    IOT_PROFILE_COUNT
} iot_profile_id_t;

typedef enum {
    IOT_CLOUD_GENERIC = 0,
    IOT_CLOUD_AWS_IOT,
    IOT_CLOUD_AZURE_IOT,
    IOT_CLOUD_GCP_MQTT
} iot_cloud_provider_t;

typedef struct {
    char device_id[IOT_DEVICE_ID_LEN];
    char fw_version[IOT_FW_VERSION_LEN];
    iot_profile_id_t profile;
    iot_cloud_provider_t cloud;
    uint32_t boot_count;
} iot_identity_t;

typedef struct {
    uint32_t seq;
    int64_t unix_ms;
    uint16_t flags;
    uint16_t payload_len;
    uint8_t payload[IOT_JSON_TELEMETRY_LEN];
} iot_telemetry_msg_t;

typedef struct {
    uint32_t seq;
    uint16_t opcode;
    uint16_t payload_len;
    uint8_t payload[IOT_JSON_SMALL_LEN];
} iot_command_msg_t;

#ifdef __cplusplus
}
#endif

#endif /* IOT_TYPES_H */
