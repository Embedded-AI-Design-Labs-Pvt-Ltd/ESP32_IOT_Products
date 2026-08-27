/**
 * @file iot_services.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Command bus: BLE/MQTT → security validate → provision/OTA/profile.
 */
#include "iot_services.h"
#include "iot_security.h"
#include "iot_provision.h"
#include "iot_ota.h"
#include "iot_log.h"
#include "iot_diag.h"
#include "iot_uc.h"

#include <string.h>
#include <stdio.h>

static const char *TAG = "iot_svc";

iot_err_t iot_services_init(void)
{
    return IOT_OK;
}

iot_err_t iot_services_fill_telemetry(iot_telemetry_msg_t *msg, const char *json)
{
    if ((msg == NULL) || (json == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    size_t n = strlen(json);
    if (n >= sizeof(msg->payload)) {
        n = sizeof(msg->payload) - 1U;
    }
    (void)memcpy(msg->payload, json, n);
    msg->payload[n] = 0;
    msg->payload_len = (uint16_t)n;
    return IOT_OK;
}

iot_err_t iot_services_handle_command(const iot_command_msg_t *cmd)
{
    iot_err_t e = iot_security_validate_command(cmd);
    if (IOT_FAIL(e)) {
        iot_diag_event(DIAG_CMD_REJECT);
        return e;
    }
    if (iot_uc_json_has_key((const char *)cmd->payload, "uc")) {
        iot_uc_out_t uco;
        e = iot_uc_dispatch_command(cmd, &uco);
        IOT_LOGI(TAG, "uc %s err=%d", iot_uc_name(uco.id), (int)uco.err);
        return e;
    }
    /* BLE config char or MQTT config topic. */
    if ((cmd->opcode == 0xFF01) || (cmd->opcode == 0x4D51 && cmd->payload[2] == 's')) {
        return iot_provision_apply_json((const char *)cmd->payload, cmd->payload_len);
    }
    if (strstr((const char *)cmd->payload, "\"ota_url\"") != NULL) {
        char url[200];
        url[0] = '\0';
        const char *p = strstr((const char *)cmd->payload, "https://");
        if (p != NULL) {
            size_t i = 0;
            while ((p[i] != '\0') && (p[i] != '"') && (i < sizeof(url) - 1U)) {
                url[i] = p[i];
                i++;
            }
            url[i] = '\0';
            return iot_ota_start(url);
        }
    }
    IOT_LOGI(TAG, "cmd opcode=0x%04x len=%u forwarded to profile", (unsigned)cmd->opcode,
             (unsigned)cmd->payload_len);
    return IOT_OK;
}
