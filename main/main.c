/**
 * @file main.c
 * @brief ESP32-C3 IoT platform startup — layered bring-up then profile.
 *
 * Boot sequence:
 *   1. NVS + logging
 *   2. Board + drivers + WDT
 *   3. Config + security identity
 *   4. Diagnostics
 *   5. FreeRTOS queues/tasks
 *   6. Profile hooks
 *   7. BLE (always, for commissioning)
 *   8. Wi-Fi if provisioned
 *   9. MQTT after WIFI_IP_READY
 *  10. Mark OTA image valid
 *
 * @syscalls nvs_flash_init, esp_ota_mark_app_valid_cancel_rollback
 * @thread app_main (ESP-IDF main task)
 */
#include "iot_core.h"
#include "iot_board.h"
#include "iot_drivers.h"
#include "iot_rtos.h"
#include "iot_tasks.h"
#include "iot_hooks.h"
#include "iot_wifi.h"
#include "iot_ble.h"
#include "iot_mqtt.h"
#include "iot_security.h"
#include "iot_ota.h"
#include "iot_diag.h"
#include "iot_provision.h"
#include "iot_services.h"
#include "iot_profile.h"
#include "iot_uc.h"
#include "iot_wdt.h"

#include <string.h>

#ifdef ESP_PLATFORM
#include "nvs_flash.h"
#include "esp_event.h"
#endif

static const char *TAG = "app_main";

static iot_err_t nvs_bootstrap(void)
{
#ifdef ESP_PLATFORM
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        e = nvs_flash_init();
    }
    return iot_err_from_esp(e);
#else
    return IOT_OK;
#endif
}

void app_main(void)
{
    IOT_LOGI(TAG, "platform %s", IOT_PLATFORM_NAME);

    if (nvs_bootstrap() != IOT_OK) {
        IOT_LOGE(TAG, "nvs fail");
        return;
    }

    (void)iot_rtos_init();
    (void)iot_config_init();
    (void)iot_board_init();
    (void)iot_drivers_init();
    (void)iot_wdt_add_current();
    (void)iot_security_init();
    (void)iot_diag_init();
    (void)iot_provision_init();
    (void)iot_services_init();
    (void)iot_uc_init();
    (void)iot_ota_init();

    iot_runtime_cfg_t cfg;
    (void)iot_config_load(&cfg);
    if (cfg.device_id[0] == '\0' || strstr(cfg.device_id, "unprovisioned") != NULL) {
        (void)iot_security_default_device_id(cfg.device_id, sizeof(cfg.device_id));
        (void)iot_config_save(&cfg);
    }

    if (iot_tasks_start(NULL) != IOT_OK) {
        IOT_LOGE(TAG, "tasks fail");
        return;
    }
    (void)iot_wdt_add_current();

    (void)iot_profile_init();
    iot_tasks_set_hooks(iot_profile_sensor_hook(), iot_profile_proc_hook(), iot_profile_cmd_hook());
    IOT_LOGI(TAG, "profile=%s device=%s", iot_profile_name(), cfg.device_id);

    (void)iot_ble_init(cfg.device_id);
    (void)iot_wifi_init();
    (void)iot_mqtt_init();

    if (cfg.provisioned && cfg.wifi_ssid[0] != '\0') {
        (void)iot_wifi_start(cfg.wifi_ssid, cfg.wifi_pass);
        iot_task_ctx_t *ctx = iot_tasks_ctx();
        uint32_t bits = iot_event_wait(ctx->system_evt, IOT_EVT_WIFI_IP, false, true, 30000U);
        if ((bits & IOT_EVT_WIFI_IP) != 0U) {
            (void)iot_mqtt_start(&cfg);
        } else {
            iot_diag_event(DIAG_WIFI_CONNECT_FAIL);
            IOT_LOGW(TAG, "wifi timeout — remain in BLE commissioning");
        }
    } else {
        IOT_LOGI(TAG, "unprovisioned — BLE commissioning");
    }

    (void)iot_ota_mark_valid();
    IOT_LOGI(TAG, "boot complete");
}
