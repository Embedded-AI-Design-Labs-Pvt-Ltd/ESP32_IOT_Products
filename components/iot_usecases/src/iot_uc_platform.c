/**
 * @file iot_uc_platform.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Platform-wide use cases: provision, OTA, identity, diag, NVS reset.
 */
#include "iot_uc_priv.h"
#include "iot_config.h"
#include "iot_diag.h"
#include "iot_provision.h"
#include "iot_ota.h"
#include "iot_security.h"
#include "iot_log.h"
#include "iot_tasks.h"
#include "iot_mqtt.h"

#include <stdio.h>
#include <string.h>

iot_err_t iot_uc_plt_factory_reset(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_runtime_cfg_t cfg;
    (void)iot_config_set_defaults(&cfg);
    iot_err_t e = iot_config_save(&cfg);
    iot_uc_out_ok(out, IOT_UC_PLT_FACTORY_RESET,
                  "{\"uc\":\"plt.factory_reset\",\"provisioned\":false}");
    out->err = e;
    return e;
}

iot_err_t iot_uc_plt_set_sample_ms(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    uint32_t ms = (uint32_t)in->i[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "sample_ms")) {
        int32_t v = 0;
        (void)iot_uc_json_i32(in->json, "sample_ms", &v);
        ms = (uint32_t)v;
    }
    if ((ms < 50U) || (ms > 60000U)) {
        iot_uc_out_err(out, IOT_UC_PLT_SET_SAMPLE_MS, IOT_ERR_INVALID_ARG);
        return IOT_ERR_INVALID_ARG;
    }
    iot_runtime_cfg_t cfg;
    (void)iot_config_load(&cfg);
    cfg.sample_period_ms = ms;
    iot_err_t e = iot_config_save(&cfg);
    char js[96];
    (void)snprintf(js, sizeof(js), "{\"uc\":\"plt.set_sample_ms\",\"ms\":%u}", (unsigned)ms);
    iot_uc_out_ok(out, IOT_UC_PLT_SET_SAMPLE_MS, js);
    out->u[0] = ms;
    out->err = e;
    return e;
}

iot_err_t iot_uc_plt_get_diag(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_err_t e = iot_diag_snapshot_json(out->json, sizeof(out->json));
    out->id = IOT_UC_PLT_GET_DIAG;
    out->err = e;
    return e;
}

iot_err_t iot_uc_plt_provision(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    const char *js = in->json != NULL ? in->json : in->s0;
    if ((js == NULL) || (js[0] == '\0')) {
        iot_uc_out_err(out, IOT_UC_PLT_PROVISION, IOT_ERR_INVALID_ARG);
        return IOT_ERR_INVALID_ARG;
    }
    iot_err_t e = iot_provision_apply_json(js, strlen(js));
    iot_uc_out_ok(out, IOT_UC_PLT_PROVISION,
                  e == IOT_OK ? "{\"uc\":\"plt.provision\",\"ok\":1}" :
                                "{\"uc\":\"plt.provision\",\"ok\":0}");
    out->err = e;
    return e;
}

iot_err_t iot_uc_plt_ota(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    const char *url = in->s1;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "ota_url")) {
        (void)iot_uc_json_str(in->json, "ota_url", out->json, 200);
        url = out->json;
        /* copy to a stable buffer */
    }
    char local[160];
    if ((url == NULL) || (url[0] == '\0')) {
        if (!iot_uc_json_str(in->json, "ota_url", local, sizeof(local))) {
            iot_uc_out_err(out, IOT_UC_PLT_OTA, IOT_ERR_INVALID_ARG);
            return IOT_ERR_INVALID_ARG;
        }
        url = local;
    } else {
        (void)snprintf(local, sizeof(local), "%s", url);
        url = local;
    }
    iot_err_t e = iot_ota_start(url);
    iot_uc_out_ok(out, IOT_UC_PLT_OTA, "{\"uc\":\"plt.ota\",\"started\":1}");
    out->err = e;
    return e;
}

iot_err_t iot_uc_plt_identity(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_identity_t id;
    (void)iot_security_load_identity(&id);
    char js[192];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"plt.identity\",\"device\":\"%s\",\"fw\":\"%s\",\"profile\":%d}",
                   id.device_id, id.fw_version, (int)id.profile);
    iot_uc_out_ok(out, IOT_UC_PLT_IDENTITY, js);
    return IOT_OK;
}

iot_err_t iot_uc_plt_set_cloud(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t c = in->i[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "cloud")) {
        (void)iot_uc_json_i32(in->json, "cloud", &c);
    }
    if ((c < 0) || (c > (int32_t)IOT_CLOUD_GCP_MQTT)) {
        iot_uc_out_err(out, IOT_UC_PLT_SET_CLOUD, IOT_ERR_INVALID_ARG);
        return IOT_ERR_INVALID_ARG;
    }
    iot_runtime_cfg_t cfg;
    (void)iot_config_load(&cfg);
    cfg.cloud = (iot_cloud_provider_t)c;
    iot_err_t e = iot_config_save(&cfg);
    char js[80];
    (void)snprintf(js, sizeof(js), "{\"uc\":\"plt.set_cloud\",\"cloud\":%d}", (int)c);
    iot_uc_out_ok(out, IOT_UC_PLT_SET_CLOUD, js);
    out->i[0] = c;
    out->err = e;
    return e;
}

iot_err_t iot_uc_plt_set_log_level(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t lvl = in->i[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "level")) {
        (void)iot_uc_json_i32(in->json, "level", &lvl);
    }
    if ((lvl < (int32_t)IOT_LOG_NONE) || (lvl > (int32_t)IOT_LOG_VERBOSE)) {
        iot_uc_out_err(out, IOT_UC_PLT_SET_LOG_LEVEL, IOT_ERR_INVALID_ARG);
        return IOT_ERR_INVALID_ARG;
    }
    iot_log_set_level((iot_log_level_t)lvl);
    char js[72];
    (void)snprintf(js, sizeof(js), "{\"uc\":\"plt.set_log_level\",\"level\":%d}", (int)lvl);
    iot_uc_out_ok(out, IOT_UC_PLT_SET_LOG_LEVEL, js);
    return IOT_OK;
}

iot_err_t iot_uc_plt_export_topics(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    char id[IOT_DEVICE_ID_LEN];
    (void)iot_security_default_device_id(id, sizeof(id));
    char t[IOT_TOPIC_LEN];
    iot_mqtt_build_topic(t, sizeof(t), id, "telemetry");
    char js[IOT_JSON_SMALL_LEN];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"plt.export_topics\",\"example\":\"%s\",\"list\":"
                   "[\"telemetry\",\"status\",\"command\",\"config\",\"diagnostics\",\"ota\"]}",
                   t);
    iot_uc_out_ok(out, IOT_UC_PLT_EXPORT_TOPICS, js);
    return IOT_OK;
}

iot_err_t iot_uc_plt_wifi_backoff(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_uc_out_ok(out, IOT_UC_PLT_WIFI_BACKOFF_POLICY,
                  "{\"uc\":\"plt.wifi_backoff\",\"min_ms\":1000,\"max_ms\":30000,\"exp\":2}");
    out->u[0] = 1000;
    out->u[1] = 30000;
    return IOT_OK;
}

iot_err_t iot_uc_plt_cmd_schema(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    iot_command_msg_t cmd;
    (void)memset(&cmd, 0, sizeof(cmd));
    if (in->json != NULL) {
        size_t n = strlen(in->json);
        if (n > sizeof(cmd.payload)) {
            n = sizeof(cmd.payload);
        }
        (void)memcpy(cmd.payload, in->json, n);
        cmd.payload_len = (uint16_t)n;
    }
    iot_err_t e = iot_security_validate_command(&cmd);
    iot_uc_out_ok(out, IOT_UC_PLT_CMD_SCHEMA, e == IOT_OK ? "{\"ok\":1}" : "{\"ok\":0}");
    out->err = e;
    return e;
}

iot_err_t iot_uc_plt_mark_ota(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_err_t e = iot_ota_mark_valid();
    iot_uc_out_ok(out, IOT_UC_PLT_MARK_OTA_VALID, "{\"uc\":\"plt.mark_ota_valid\"}");
    out->err = e;
    return e;
}

iot_err_t iot_uc_plt_shutdown(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_tasks_request_shutdown();
    iot_uc_out_ok(out, IOT_UC_PLT_SHUTDOWN, "{\"uc\":\"plt.shutdown\"}");
    return IOT_OK;
}
