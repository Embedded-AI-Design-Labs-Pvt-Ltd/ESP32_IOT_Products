/**
 * @file iot_ble.c
 * @brief NimBLE GATT: Device Info, Config, Sensor, Control, Diagnostics, OTA status.
 *
 * Heavy work is posted to command_q. GAP/GATT callbacks return immediately.
 *
 * @syscalls nimble_port_*, ble_gap_adv_start, ble_gatts_*
 * @thread nimble host task + GAP event
 */
#include "iot_ble.h"
#include "iot_log.h"
#include "iot_tasks.h"
#include "iot_config.h"
#include "iot_version.h"

#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_bt.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#endif

static const char *TAG = "iot_ble";
static iot_ble_state_t s_state = IOT_BLE_IDLE;
static bool s_connected;
static char s_name[32] = "ESP32C3-IoT";
static uint8_t s_sensor_cache[64];
static uint16_t s_sensor_len;

#ifdef ESP_PLATFORM
static uint16_t s_conn;
static uint16_t s_chr_sensor_val;

/* 128-bit service UUID: 0000IOT1-0000-1000-8000-00805F9B34FB encoded little-endian. */
static const ble_uuid128_t s_svc_uuid =
    BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x01, 0x49, 0x4f, 0x54);

static int gatt_access(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_chr_def s_chrs[] = {
    {
        /* Config characteristic — write provisioning JSON (SSID etc). */
        .uuid = BLE_UUID16_DECLARE(0xFF01),
        .access_cb = gatt_access,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        .arg = (void *)(uintptr_t)0xFF01,
    },
    {
        .uuid = BLE_UUID16_DECLARE(0xFF02),
        .access_cb = gatt_access,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &s_chr_sensor_val,
        .arg = (void *)(uintptr_t)0xFF02,
    },
    {
        .uuid = BLE_UUID16_DECLARE(0xFF03),
        .access_cb = gatt_access,
        .flags = BLE_GATT_CHR_F_WRITE,
        .arg = (void *)(uintptr_t)0xFF03,
    },
    {
        .uuid = BLE_UUID16_DECLARE(0xFF04),
        .access_cb = gatt_access,
        .flags = BLE_GATT_CHR_F_READ,
        .arg = (void *)(uintptr_t)0xFF04,
    },
    {
        .uuid = BLE_UUID16_DECLARE(0xFF05),
        .access_cb = gatt_access,
        .flags = BLE_GATT_CHR_F_READ,
        .arg = (void *)(uintptr_t)0xFF05,
    },
    {0},
};

static const struct ble_gatt_svc_def s_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &s_svc_uuid.u,
        .characteristics = s_chrs,
    },
    {0},
};

static int gatt_access(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn;
    (void)attr;
    uint16_t uuid16 = (uint16_t)(uintptr_t)arg;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        const char *p = "ok";
        uint16_t n = 2;
        if (uuid16 == 0xFF02) {
            p = (const char *)s_sensor_cache;
            n = s_sensor_len == 0U ? 2U : s_sensor_len;
        } else if (uuid16 == 0xFF04) {
            p = "diag:ok";
            n = 7;
        } else if (uuid16 == 0xFF05) {
            p = IOT_FW_VERSION_STRING;
            n = (uint16_t)strlen(IOT_FW_VERSION_STRING);
        } else if (uuid16 == 0xFF01) {
            p = iot_config_get()->provisioned ? "prov=1" : "prov=0";
            n = (uint16_t)strlen(p);
        }
        int rc = os_mbuf_append(ctxt->om, p, n);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        /* Copy into command queue — do not parse JSON here. */
        iot_command_msg_t cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.opcode = uuid16;
        uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
        if (om_len > sizeof(cmd.payload)) {
            om_len = (uint16_t)sizeof(cmd.payload);
        }
        cmd.payload_len = om_len;
        (void)ble_hs_mbuf_to_flat(ctxt->om, cmd.payload, om_len, NULL);
        iot_task_ctx_t *ctx = iot_tasks_ctx();
        if ((ctx != NULL) && (ctx->command_q != NULL)) {
            (void)iot_queue_send(ctx->command_q, &cmd, 0U);
        }
        return 0;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static void ble_on_sync(void)
{
    (void)ble_hs_util_ensure_addr(0);
    (void)iot_ble_start_advertising();
}

static int gap_event(struct ble_gap_event *ev, void *arg)
{
    (void)arg;
    switch (ev->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (ev->connect.status == 0) {
            s_conn = ev->connect.conn_handle;
            s_connected = true;
            s_state = IOT_BLE_CONNECTED;
            iot_task_ctx_t *ctx = iot_tasks_ctx();
            if ((ctx != NULL) && (ctx->system_evt != NULL)) {
                iot_event_set(ctx->system_evt, IOT_EVT_BLE_CONN);
            }
        } else {
            s_state = IOT_BLE_ADVERTISING;
            (void)iot_ble_start_advertising();
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        s_connected = false;
        s_state = IOT_BLE_ADVERTISING;
        (void)iot_ble_start_advertising();
        break;
    default:
        break;
    }
    return 0;
}

static void nimble_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}
#endif /* ESP_PLATFORM */

iot_err_t iot_ble_init(const char *device_name)
{
    if (device_name != NULL) {
        (void)strncpy(s_name, device_name, sizeof(s_name) - 1U);
    }
#ifdef ESP_PLATFORM
    esp_err_t err = nimble_port_init();
    if (err != ESP_OK) {
        return iot_err_from_esp(err);
    }
    ble_hs_cfg.sync_cb = ble_on_sync;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_mitm = 1;
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_svc_gap_init();
    ble_svc_gatt_init();
    (void)ble_svc_gap_device_name_set(s_name);
    (void)ble_gatts_count_cfg(s_svcs);
    (void)ble_gatts_add_svcs(s_svcs);
    nimble_port_freertos_init(nimble_host_task);
    s_state = IOT_BLE_ADVERTISING;
#else
    s_state = IOT_BLE_ADVERTISING;
#endif
    IOT_LOGI(TAG, "ble init name=%s", s_name);
    return IOT_OK;
}

iot_err_t iot_ble_start_advertising(void)
{
#ifdef ESP_PLATFORM
    struct ble_gap_adv_params ap;
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)s_name;
    fields.name_len = (uint8_t)strlen(s_name);
    fields.name_is_complete = 1;
    (void)ble_gap_adv_set_fields(&fields);
    memset(&ap, 0, sizeof(ap));
    ap.conn_mode = BLE_GAP_CONN_MODE_UND;
    ap.disc_mode = BLE_GAP_DISC_MODE_GEN;
    int rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &ap, gap_event, NULL);
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        IOT_LOGW(TAG, "adv start rc=%d", rc);
        return IOT_ERR_FAIL;
    }
#endif
    s_state = IOT_BLE_ADVERTISING;
    return IOT_OK;
}

iot_err_t iot_ble_stop_advertising(void)
{
#ifdef ESP_PLATFORM
    (void)ble_gap_adv_stop();
#endif
    s_state = IOT_BLE_IDLE;
    return IOT_OK;
}

iot_ble_state_t iot_ble_state(void)
{
    return s_state;
}

bool iot_ble_is_connected(void)
{
    return s_connected;
}

iot_err_t iot_ble_notify_sensor(const uint8_t *data, uint16_t len)
{
    if ((data == NULL) || (len == 0U)) {
        return IOT_ERR_INVALID_ARG;
    }
    if (len > sizeof(s_sensor_cache)) {
        len = (uint16_t)sizeof(s_sensor_cache);
    }
#ifdef ESP_PLATFORM
    memcpy(s_sensor_cache, data, len);
    s_sensor_len = len;
    if (!s_connected) {
        return IOT_OK;
    }
    struct os_mbuf *om = ble_hs_mbuf_from_flat(data, len);
    if (om == NULL) {
        return IOT_ERR_NO_MEM;
    }
    int rc = ble_gatts_notify_custom(s_conn, s_chr_sensor_val, om);
    return (rc == 0) ? IOT_OK : IOT_ERR_FAIL;
#else
    (void)data;
    (void)len;
    return IOT_OK;
#endif
}
