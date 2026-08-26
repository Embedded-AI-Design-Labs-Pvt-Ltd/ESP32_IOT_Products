/**
 * @file iot_provision.c
 * @brief Minimal JSON field extractor for BLE commissioning payloads.
 *
 * Expected keys: ssid, pass, mqtt, user, mpw, cloud (integers 0-3).
 */
#include "iot_provision.h"
#include "iot_log.h"
#include "iot_security.h"
#include "iot_tasks.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *TAG = "iot_prov";

static bool extract_str(const char *json, const char *key, char *out, size_t out_len)
{
    char pat[32];
    (void)snprintf(pat, sizeof(pat), "\"%s\"", key);
    const char *p = strstr(json, pat);
    if (p == NULL) {
        return false;
    }
    p = strchr(p + strlen(pat), ':');
    if (p == NULL) {
        return false;
    }
    p++;
    while (*p == ' ') {
        p++;
    }
    if (*p != '"') {
        return false;
    }
    p++;
    size_t i = 0;
    while ((p[i] != '\0') && (p[i] != '"') && (i + 1U < out_len)) {
        out[i] = p[i];
        i++;
    }
    out[i] = '\0';
    return i > 0U;
}

iot_err_t iot_provision_init(void)
{
    return IOT_OK;
}

bool iot_provision_is_complete(void)
{
    const iot_runtime_cfg_t *c = iot_config_get();
    return (c != NULL) && c->provisioned && (c->wifi_ssid[0] != '\0');
}

iot_err_t iot_provision_apply_json(const char *json, size_t len)
{
    if ((json == NULL) || (len == 0U) || (len > 512U)) {
        return IOT_ERR_INVALID_ARG;
    }
    iot_runtime_cfg_t cfg;
    (void)iot_config_load(&cfg);
    (void)extract_str(json, "ssid", cfg.wifi_ssid, sizeof(cfg.wifi_ssid));
    (void)extract_str(json, "pass", cfg.wifi_pass, sizeof(cfg.wifi_pass));
    (void)extract_str(json, "mqtt", cfg.mqtt_uri, sizeof(cfg.mqtt_uri));
    (void)extract_str(json, "user", cfg.mqtt_user, sizeof(cfg.mqtt_user));
    (void)extract_str(json, "mpw", cfg.mqtt_pass, sizeof(cfg.mqtt_pass));
    if (cfg.device_id[0] == '\0' || strstr(cfg.device_id, "unprovisioned") != NULL) {
        (void)iot_security_default_device_id(cfg.device_id, sizeof(cfg.device_id));
    }
    cfg.provisioned = (cfg.wifi_ssid[0] != '\0');
    iot_err_t e = iot_config_save(&cfg);
    if (IOT_SUCCESS(e) && cfg.provisioned) {
        iot_task_ctx_t *ctx = iot_tasks_ctx();
        if ((ctx != NULL) && (ctx->system_evt != NULL)) {
            iot_event_set(ctx->system_evt, IOT_EVT_PROVISIONED);
        }
        IOT_LOGI(TAG, "provisioned ssid=%s", cfg.wifi_ssid);
    }
    return e;
}
