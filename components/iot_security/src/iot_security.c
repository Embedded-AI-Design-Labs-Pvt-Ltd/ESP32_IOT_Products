/**
 * @file iot_security.c
 * @brief Identity from MAC; command size/schema gate; TLS via IDF cert bundle.
 *
 * Production certs live in NVS or flash partitions — never in this file.
 */
#include "iot_security.h"
#include "iot_log.h"
#include "iot_config.h"
#include "iot_version.h"

#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_mac.h"
#include "esp_system.h"
#endif

static const char *TAG = "iot_sec";
static iot_identity_t s_id;

iot_err_t iot_security_default_device_id(char *out, size_t out_len)
{
    if ((out == NULL) || (out_len < 18U)) {
        return IOT_ERR_INVALID_ARG;
    }
#ifdef ESP_PLATFORM
    uint8_t mac[6];
    esp_err_t e = esp_read_mac(mac, ESP_MAC_WIFI_STA);
    if (e != ESP_OK) {
        return iot_err_from_esp(e);
    }
    (void)snprintf(out, out_len, "c3-%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#else
    (void)snprintf(out, out_len, "c3-hostsim000000");
#endif
    return IOT_OK;
}

iot_err_t iot_security_init(void)
{
    (void)memset(&s_id, 0, sizeof(s_id));
    (void)iot_security_default_device_id(s_id.device_id, sizeof(s_id.device_id));
    (void)memcpy(s_id.fw_version, IOT_FW_VERSION_STRING, sizeof(IOT_FW_VERSION_STRING));
#ifdef CONFIG_IOT_PROFILE_ENERGY
    s_id.profile = IOT_PROFILE_ENERGY;
#elif defined(CONFIG_IOT_PROFILE_INDUSTRIAL)
    s_id.profile = IOT_PROFILE_INDUSTRIAL;
#elif defined(CONFIG_IOT_PROFILE_PREDICTIVE)
    s_id.profile = IOT_PROFILE_PREDICTIVE;
#elif defined(CONFIG_IOT_PROFILE_WATER)
    s_id.profile = IOT_PROFILE_WATER;
#elif defined(CONFIG_IOT_PROFILE_EV_CHARGER)
    s_id.profile = IOT_PROFILE_EV_CHARGER;
#elif defined(CONFIG_IOT_PROFILE_AUTOMOTIVE)
    s_id.profile = IOT_PROFILE_AUTOMOTIVE;
#elif defined(CONFIG_IOT_PROFILE_ASSET_TRACKER)
    s_id.profile = IOT_PROFILE_ASSET_TRACKER;
#elif defined(CONFIG_IOT_PROFILE_AGRICULTURE)
    s_id.profile = IOT_PROFILE_AGRICULTURE;
#elif defined(CONFIG_IOT_PROFILE_COLDCHAIN)
    s_id.profile = IOT_PROFILE_COLDCHAIN;
#elif defined(CONFIG_IOT_PROFILE_ROBOTICS)
    s_id.profile = IOT_PROFILE_ROBOTICS;
#else
    s_id.profile = IOT_PROFILE_ENERGY;
#endif
    IOT_LOGI(TAG, "identity %s fw=%s", s_id.device_id, s_id.fw_version);
    IOT_LOGI(TAG, "TLS: use certificate bundle; do not embed production keys");
    return IOT_OK;
}

iot_err_t iot_security_load_identity(iot_identity_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    *out = s_id;
    return IOT_OK;
}

iot_err_t iot_security_validate_command(const iot_command_msg_t *cmd)
{
    if (cmd == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (cmd->payload_len > sizeof(cmd->payload)) {
        return IOT_ERR_OVERFLOW;
    }
    /* Reject empty actuator payloads and non-JSON probes. */
    if (cmd->payload_len == 0U) {
        return IOT_ERR_DENIED;
    }
    if ((cmd->payload[0] != '{') && (cmd->payload[0] != '[') && (cmd->opcode != 0xFF01) &&
        (cmd->opcode != 0xFF03)) {
        return IOT_ERR_PROTOCOL;
    }
    return IOT_OK;
}
