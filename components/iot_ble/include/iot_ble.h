/**
 * @file iot_ble.h
 * @brief Generic IoT GATT server (NimBLE). Callbacks only enqueue work.
 */
#ifndef IOT_BLE_H
#define IOT_BLE_H

#include "iot_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_BLE_IDLE = 0,
    IOT_BLE_ADVERTISING,
    IOT_BLE_CONNECTED,
    IOT_BLE_PAIRED
} iot_ble_state_t;

iot_err_t iot_ble_init(const char *device_name);
iot_err_t iot_ble_start_advertising(void);
iot_err_t iot_ble_stop_advertising(void);
iot_ble_state_t iot_ble_state(void);
iot_err_t iot_ble_notify_sensor(const uint8_t *data, uint16_t len);
bool iot_ble_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_BLE_H */
