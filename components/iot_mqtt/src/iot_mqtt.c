/**
 * @file iot_mqtt.c
 * @brief MQTT task: telemetry_q → broker; inbound command → command_q.
 *
 * Topics:
 *   device/{id}/telemetry|status|command|config|diagnostics|ota
 *
 * @syscalls esp_mqtt_client_*
 * @thread mqtt_io (IDF) + platform mqtt pump task
 */
#include "iot_mqtt.h"
#include "iot_log.h"
#include "iot_tasks.h"
#include "iot_rtos.h"

#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "mqtt_client.h"
#include "esp_crt_bundle.h"
static esp_mqtt_client_handle_t s_client;
#endif

static const char *TAG = "iot_mqtt";
static iot_mqtt_state_t s_state = IOT_MQTT_DOWN;
static char s_dev[IOT_DEVICE_ID_LEN];
static iot_task_t s_pump;
static bool s_run;

void iot_mqtt_build_topic(char *out, size_t out_len, const char *device_id, const char *suffix)
{
    if ((out == NULL) || (out_len == 0U)) {
        return;
    }
    (void)snprintf(out, out_len, "device/%s/%s", device_id != NULL ? device_id : "unknown",
                   suffix != NULL ? suffix : "telemetry");
}

#ifdef ESP_PLATFORM
static void mqtt_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    (void)base;
    esp_mqtt_event_handle_t ev = (esp_mqtt_event_handle_t)data;
    switch (id) {
    case MQTT_EVENT_CONNECTED: {
        s_state = IOT_MQTT_UP;
        char t[IOT_TOPIC_LEN];
        iot_mqtt_build_topic(t, sizeof(t), s_dev, "command");
        (void)esp_mqtt_client_subscribe(s_client, t, 1);
        iot_mqtt_build_topic(t, sizeof(t), s_dev, "config");
        (void)esp_mqtt_client_subscribe(s_client, t, 1);
        iot_mqtt_build_topic(t, sizeof(t), s_dev, "ota");
        (void)esp_mqtt_client_subscribe(s_client, t, 1);
        iot_task_ctx_t *ctx = iot_tasks_ctx();
        if ((ctx != NULL) && (ctx->system_evt != NULL)) {
            iot_event_set(ctx->system_evt, IOT_EVT_MQTT_UP);
        }
        IOT_LOGI(TAG, "connected + subscribed");
        break;
    }
    case MQTT_EVENT_DISCONNECTED:
        s_state = IOT_MQTT_CONNECTING;
        iot_task_ctx_t *ctx = iot_tasks_ctx();
        if ((ctx != NULL) && (ctx->system_evt != NULL)) {
            iot_event_clear(ctx->system_evt, IOT_EVT_MQTT_UP);
        }
        break;
    case MQTT_EVENT_DATA: {
        iot_command_msg_t cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.opcode = 0x4D51; /* 'MQ' */
        int n = ev->data_len;
        if (n > (int)sizeof(cmd.payload)) {
            n = (int)sizeof(cmd.payload);
        }
        cmd.payload_len = (uint16_t)n;
        if (ev->data != NULL && n > 0) {
            memcpy(cmd.payload, ev->data, (size_t)n);
        }
        iot_task_ctx_t *c = iot_tasks_ctx();
        if ((c != NULL) && (c->command_q != NULL)) {
            (void)iot_queue_send(c->command_q, &cmd, 0U);
        }
        break;
    }
    case MQTT_EVENT_ERROR:
        s_state = IOT_MQTT_ERROR;
        break;
    default:
        break;
    }
}
#endif

static void mqtt_pump(void *arg)
{
    (void)arg;
    iot_telemetry_msg_t msg;
    iot_task_ctx_t *ctx = iot_tasks_ctx();
    IOT_LOGI(TAG, "mqtt pump task");
    while (s_run) {
        if ((ctx == NULL) || (ctx->telemetry_q == NULL)) {
            iot_task_delay_ms(100U);
            ctx = iot_tasks_ctx();
            continue;
        }
        if (iot_queue_recv(ctx->telemetry_q, &msg, 250U) != IOT_OK) {
            continue;
        }
        (void)iot_mqtt_publish_telemetry(&msg);
    }
}

iot_err_t iot_mqtt_init(void)
{
    s_state = IOT_MQTT_DOWN;
    return IOT_OK;
}

iot_err_t iot_mqtt_start(const iot_runtime_cfg_t *cfg)
{
    if (cfg == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    (void)memcpy(s_dev, cfg->device_id, IOT_DEVICE_ID_LEN);
#ifdef ESP_PLATFORM
    esp_mqtt_client_config_t mc = {0};
    mc.broker.address.uri = cfg->mqtt_uri;
    mc.credentials.client_id = cfg->device_id;
    if (cfg->mqtt_user[0] != '\0') {
        mc.credentials.username = cfg->mqtt_user;
        mc.credentials.authentication.password = cfg->mqtt_pass;
    }
    mc.session.keepalive = 30;
    mc.network.reconnect_timeout_ms = 5000;
    mc.broker.verification.crt_bundle_attach = esp_crt_bundle_attach;
    s_client = esp_mqtt_client_init(&mc);
    if (s_client == NULL) {
        return IOT_ERR_NO_MEM;
    }
    (void)esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event, NULL);
    s_state = IOT_MQTT_CONNECTING;
    esp_err_t err = esp_mqtt_client_start(s_client);
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
#else
    s_state = IOT_MQTT_UP;
#endif
    s_run = true;
    return iot_task_create("mqtt_pump", mqtt_pump, NULL, 4096U, IOT_PRIO_MQTT, &s_pump);
}

iot_err_t iot_mqtt_stop(void)
{
    s_run = false;
#ifdef ESP_PLATFORM
    if (s_client != NULL) {
        (void)esp_mqtt_client_stop(s_client);
        (void)esp_mqtt_client_destroy(s_client);
        s_client = NULL;
    }
#endif
    s_state = IOT_MQTT_DOWN;
    return IOT_OK;
}

iot_err_t iot_mqtt_publish_telemetry(const iot_telemetry_msg_t *msg)
{
    if (msg == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    char topic[IOT_TOPIC_LEN];
    iot_mqtt_build_topic(topic, sizeof(topic), s_dev, "telemetry");
#ifdef ESP_PLATFORM
    if ((s_client == NULL) || (s_state != IOT_MQTT_UP)) {
        return IOT_ERR_INVALID_STATE;
    }
    int mid = esp_mqtt_client_publish(s_client, topic, (const char *)msg->payload, (int)msg->payload_len, 1, 0);
    return (mid >= 0) ? IOT_OK : IOT_ERR_FAIL;
#else
    IOT_LOGD(TAG, "sim pub %s len=%u", topic, (unsigned)msg->payload_len);
    return IOT_OK;
#endif
}

iot_mqtt_state_t iot_mqtt_state(void)
{
    return s_state;
}
