/**
 * @file iot_wifi.h
 * @brief Thread-safe Wi-Fi station manager with reconnect/backoff.
 */
#ifndef IOT_WIFI_H
#define IOT_WIFI_H

#include "iot_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WIFI_DISCONNECTED = 0,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_IP_READY,
    WIFI_ERROR,
    WIFI_RECONNECTING
} iot_wifi_state_t;

typedef struct {
    char ip[16];
    int rssi;
    uint32_t retry_count;
    iot_wifi_state_t state;
} iot_wifi_status_t;

iot_err_t iot_wifi_init(void);
iot_err_t iot_wifi_start(const char *ssid, const char *pass);
iot_err_t iot_wifi_stop(void);
iot_wifi_state_t iot_wifi_state(void);
iot_err_t iot_wifi_get_status(iot_wifi_status_t *out);

#ifdef __cplusplus
}
#endif

#endif /* IOT_WIFI_H */
