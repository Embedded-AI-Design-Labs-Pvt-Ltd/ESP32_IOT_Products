/**
 * @file iot_config.c
 * @brief Runtime configuration. NVS on target; RAM file-backed image on host.
 *
 * @ownership Single mutex `s_lock` owned by this module.
 * @thread_safety All public APIs take `s_lock`.
 */
#include "iot_config.h"
#include "iot_log.h"

#include <string.h>

#ifdef ESP_PLATFORM
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#else
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#endif

static const char *TAG = "iot_cfg";
static iot_runtime_cfg_t s_cfg;
static bool s_ready;

#ifdef ESP_PLATFORM
static SemaphoreHandle_t s_lock;
#define LOCK()   (void)xSemaphoreTake(s_lock, portMAX_DELAY)
#define UNLOCK() (void)xSemaphoreGive(s_lock)
#else
#ifdef _WIN32
static CRITICAL_SECTION s_lock;
#define LOCK()   EnterCriticalSection(&s_lock)
#define UNLOCK() LeaveCriticalSection(&s_lock)
#else
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
#define LOCK()   (void)pthread_mutex_lock(&s_lock)
#define UNLOCK() (void)pthread_mutex_unlock(&s_lock)
#endif
#endif

iot_err_t iot_config_set_defaults(iot_runtime_cfg_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)memset(out, 0, sizeof(*out));
    (void)memcpy(out->device_id, "esp32c3-unprovisioned", 22);
    (void)memcpy(out->mqtt_uri, "mqtts://mqtt.local:8883", 24);
    out->sample_period_ms = 1000U;
    out->cloud = IOT_CLOUD_GENERIC;
    out->provisioned = false;
    return IOT_OK;
}

iot_err_t iot_config_init(void)
{
#ifdef ESP_PLATFORM
    if (s_lock == NULL) {
        s_lock = xSemaphoreCreateMutex();
        if (s_lock == NULL) {
            return IOT_ERR_NO_MEM;
        }
    }
#else
#ifdef _WIN32
    static bool cs_init;
    if (!cs_init) {
        InitializeCriticalSection(&s_lock);
        cs_init = true;
    }
#endif
#endif
    LOCK();
    if (s_ready) {
        UNLOCK();
        return IOT_ERR_ALREADY_INIT;
    }
    (void)iot_config_set_defaults(&s_cfg);
    s_ready = true;
    UNLOCK();
    IOT_LOGI(TAG, "config init");
    return IOT_OK;
}

iot_err_t iot_config_load(iot_runtime_cfg_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (!s_ready) {
        return IOT_ERR_NOT_INIT;
    }

#ifdef ESP_PLATFORM
    nvs_handle_t h;
    esp_err_t e = nvs_open(IOT_CFG_NS, NVS_READONLY, &h);
    if (e != ESP_OK) {
        LOCK();
        *out = s_cfg;
        UNLOCK();
        return IOT_OK;
    }
    iot_runtime_cfg_t tmp;
    (void)iot_config_set_defaults(&tmp);
    size_t len = sizeof(tmp.device_id);
    (void)nvs_get_str(h, IOT_CFG_KEY_DEVICE_ID, tmp.device_id, &len);
    len = sizeof(tmp.wifi_ssid);
    (void)nvs_get_str(h, IOT_CFG_KEY_WIFI_SSID, tmp.wifi_ssid, &len);
    len = sizeof(tmp.wifi_pass);
    (void)nvs_get_str(h, IOT_CFG_KEY_WIFI_PASS, tmp.wifi_pass, &len);
    len = sizeof(tmp.mqtt_uri);
    (void)nvs_get_str(h, IOT_CFG_KEY_MQTT_URI, tmp.mqtt_uri, &len);
    len = sizeof(tmp.mqtt_user);
    (void)nvs_get_str(h, IOT_CFG_KEY_MQTT_USER, tmp.mqtt_user, &len);
    len = sizeof(tmp.mqtt_pass);
    (void)nvs_get_str(h, IOT_CFG_KEY_MQTT_PASS, tmp.mqtt_pass, &len);
    (void)nvs_get_u32(h, IOT_CFG_KEY_SAMPLE_MS, &tmp.sample_period_ms);
    uint8_t prov = 0;
    (void)nvs_get_u8(h, IOT_CFG_KEY_PROVISIONED, &prov);
    tmp.provisioned = (prov != 0U);
    uint8_t cloud = 0;
    (void)nvs_get_u8(h, IOT_CFG_KEY_CLOUD, &cloud);
    tmp.cloud = (iot_cloud_provider_t)cloud;
    nvs_close(h);
    LOCK();
    s_cfg = tmp;
    *out = s_cfg;
    UNLOCK();
    return IOT_OK;
#else
    LOCK();
    *out = s_cfg;
    UNLOCK();
    return IOT_OK;
#endif
}

iot_err_t iot_config_save(const iot_runtime_cfg_t *in)
{
    if (in == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (!s_ready) {
        return IOT_ERR_NOT_INIT;
    }

#ifdef ESP_PLATFORM
    nvs_handle_t h;
    esp_err_t e = nvs_open(IOT_CFG_NS, NVS_READWRITE, &h);
    if (e != ESP_OK) {
        return iot_err_from_esp(e);
    }
    (void)nvs_set_str(h, IOT_CFG_KEY_DEVICE_ID, in->device_id);
    (void)nvs_set_str(h, IOT_CFG_KEY_WIFI_SSID, in->wifi_ssid);
    (void)nvs_set_str(h, IOT_CFG_KEY_WIFI_PASS, in->wifi_pass);
    (void)nvs_set_str(h, IOT_CFG_KEY_MQTT_URI, in->mqtt_uri);
    (void)nvs_set_str(h, IOT_CFG_KEY_MQTT_USER, in->mqtt_user);
    (void)nvs_set_str(h, IOT_CFG_KEY_MQTT_PASS, in->mqtt_pass);
    (void)nvs_set_u32(h, IOT_CFG_KEY_SAMPLE_MS, in->sample_period_ms);
    (void)nvs_set_u8(h, IOT_CFG_KEY_PROVISIONED, in->provisioned ? 1U : 0U);
    (void)nvs_set_u8(h, IOT_CFG_KEY_CLOUD, (uint8_t)in->cloud);
    (void)nvs_commit(h);
    nvs_close(h);
#endif
    LOCK();
    s_cfg = *in;
    UNLOCK();
    IOT_LOGI(TAG, "config saved provisioned=%d", (int)in->provisioned);
    return IOT_OK;
}

const iot_runtime_cfg_t *iot_config_get(void)
{
    return &s_cfg;
}
