/**
 * @file test_runner.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Host unit tests (no ESP32 hardware). Compile with tests/host/CMakeLists.txt
 *
 * Test ID format: UT-<MODULE>-<NN>
 */
#include "iot_err.h"
#include "iot_config.h"
#include "iot_gpio.h"
#include "iot_rtos.h"
#include "iot_security.h"
#include "iot_diag.h"
#include "iot_provision.h"
#include "iot_services.h"
#include "iot_mqtt.h"
#include "iot_profile.h"
#include "iot_uc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_fail;
static int g_pass;

#define EXPECT(id, cond)                                                                 \
    do {                                                                                 \
        if (cond) {                                                                      \
            g_pass++;                                                                    \
            printf("PASS %s\n", id);                                                     \
        } else {                                                                         \
            g_fail++;                                                                    \
            printf("FAIL %s\n", id);                                                     \
        }                                                                                \
    } while (0)

int main(void)
{
    printf("ESP32-C3 IoT platform host tests\n");

    EXPECT("UT-ERR-01", strcmp(iot_err_to_name(IOT_OK), "IOT_OK") == 0);
    EXPECT("UT-ERR-02", IOT_FAIL(IOT_ERR_TIMEOUT));

    EXPECT("UT-CFG-01", iot_config_init() == IOT_OK);
    iot_runtime_cfg_t cfg;
    EXPECT("UT-CFG-02", iot_config_set_defaults(&cfg) == IOT_OK);
    EXPECT("UT-CFG-03", cfg.provisioned == false);
    (void)memcpy(cfg.wifi_ssid, "lab", 4);
    EXPECT("UT-CFG-04", iot_config_save(&cfg) == IOT_OK);

    EXPECT("UT-RTOS-01", iot_rtos_init() == IOT_OK);
    iot_mutex_t m = NULL;
    EXPECT("UT-RTOS-02", iot_mutex_create(&m) == IOT_OK);
    EXPECT("UT-RTOS-03", iot_mutex_lock(m, 100) == IOT_OK);
    iot_mutex_unlock(m);
    iot_sem_t s = NULL;
    EXPECT("UT-RTOS-04", iot_sem_binary_create(&s) == IOT_OK);
    iot_sem_give(s);
    EXPECT("UT-RTOS-05", iot_sem_take(s, 50) == IOT_OK);
    iot_queue_t q = NULL;
    EXPECT("UT-RTOS-06", iot_queue_create(4, sizeof(int), &q) == IOT_OK);
    int v = 7;
    EXPECT("UT-RTOS-07", iot_queue_send(q, &v, 10) == IOT_OK);
    int r = 0;
    EXPECT("UT-RTOS-08", iot_queue_recv(q, &r, 10) == IOT_OK && r == 7);

    EXPECT("UT-GPIO-01", iot_gpio_init() == IOT_OK);
    EXPECT("UT-GPIO-02", iot_gpio_config(8, IOT_GPIO_OUT) == IOT_OK);
    EXPECT("UT-GPIO-03", iot_gpio_write(8, true) == IOT_OK);
    bool lvl = false;
    EXPECT("UT-GPIO-04", iot_gpio_read(8, &lvl) == IOT_OK && lvl == true);

    EXPECT("UT-SEC-01", iot_security_init() == IOT_OK);
    iot_command_msg_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    EXPECT("UT-SEC-02", iot_security_validate_command(&cmd) == IOT_ERR_DENIED);
    cmd.payload[0] = '{';
    cmd.payload_len = 2;
    cmd.payload[1] = '}';
    EXPECT("UT-SEC-03", iot_security_validate_command(&cmd) == IOT_OK);

    EXPECT("UT-DIAG-01", iot_diag_init() == IOT_OK);
    iot_diag_event(DIAG_WIFI_CONNECT_FAIL);
    iot_diag_snapshot_t snap;
    EXPECT("UT-DIAG-02", iot_diag_snapshot(&snap) == IOT_OK && snap.wifi_fails >= 1);

    EXPECT("UT-PROV-01", iot_provision_init() == IOT_OK);
    const char *js = "{\"ssid\":\"labnet\",\"pass\":\"secret12\",\"mqtt\":\"mqtts://b:8883\"}";
    EXPECT("UT-PROV-02", iot_provision_apply_json(js, strlen(js)) == IOT_OK);
    EXPECT("UT-PROV-03", iot_provision_is_complete());

    char topic[64];
    iot_mqtt_build_topic(topic, sizeof(topic), "c3-1", "telemetry");
    EXPECT("UT-MQTT-01", strcmp(topic, "device/c3-1/telemetry") == 0);

    EXPECT("UT-PROF-01", iot_profile_energy_init() == IOT_OK);
    iot_telemetry_msg_t tel;
    memset(&tel, 0, sizeof(tel));
    EXPECT("UT-PROF-02", iot_profile_energy_sensor(&tel) == IOT_OK && tel.payload_len > 0);

    EXPECT("UT-UC-01", iot_uc_init() == IOT_OK);
    EXPECT("UT-UC-02", iot_uc_count() >= 70);
    EXPECT("UT-UC-03", iot_uc_id_from_name("energy.measure") == IOT_UC_ENERGY_MEASURE);
    EXPECT("UT-UC-04", iot_uc_cpp_catalog_size() == iot_uc_count());

    iot_uc_out_t uo;
    EXPECT("UT-UC-EN-01", iot_uc_run_json("{\"uc\":\"energy.measure\",\"v\":230,\"i\":1.0}", 42, &uo) == IOT_OK);
    EXPECT("UT-UC-EN-02", uo.f[2] > 100.0f);
    EXPECT("UT-UC-EN-03", iot_uc_run_json("{\"uc\":\"energy.set_limits\",\"over_w\":100}", 42, &uo) == IOT_OK);
    EXPECT("UT-UC-EN-04", iot_uc_run_json("{\"uc\":\"energy.over_power\"}", 27, &uo) == IOT_OK && uo.alarm);

    EXPECT("UT-UC-W-01", iot_uc_run_json("{\"uc\":\"water.valve_close\"}", 28, &uo) == IOT_OK);
    EXPECT("UT-UC-W-02", iot_uc_water_valve_is_open() == false);
    EXPECT("UT-UC-W-03", iot_uc_run_json("{\"uc\":\"water.valve_open\"}", 27, &uo) == IOT_OK);

    EXPECT("UT-UC-EV-01", iot_uc_run_json("{\"uc\":\"ev.reject_contactor\"}", 30, &uo) == IOT_ERR_DENIED);
    EXPECT("UT-UC-EV-02", iot_uc_run_json("{\"uc\":\"ev.start\"}", 18, &uo) == IOT_OK);

    EXPECT("UT-UC-AUTO-01", iot_uc_run_json("{\"uc\":\"auto.reject_tx\"}", 25, &uo) == IOT_ERR_DENIED);
    EXPECT("UT-UC-RB-01", iot_uc_run_json("{\"uc\":\"robot.reject_motion\"}", 30, &uo) == IOT_ERR_DENIED);

    EXPECT("UT-UC-IND-01", iot_uc_run_json("{\"uc\":\"industrial.sample\",\"t\":90,\"vib\":1,\"i\":1,\"p_bar\":1}", 64, &uo) == IOT_OK);
    EXPECT("UT-UC-IND-02", strstr(uo.json, "FAULT") != NULL);
    EXPECT("UT-UC-IND-03", iot_uc_run_json("{\"uc\":\"industrial.ack\"}", 25, &uo) == IOT_OK);

    EXPECT("UT-UC-PM-01", iot_uc_run_json("{\"uc\":\"predictive.features\",\"accel\":3.0}", 44, &uo) == IOT_OK);
    EXPECT("UT-UC-AG-01", iot_uc_run_json("{\"uc\":\"agri.soil\",\"moist\":10}", 32, &uo) == IOT_OK);
    EXPECT("UT-UC-CC-01", iot_uc_run_json("{\"uc\":\"cold.sample\",\"t_c\":12,\"rh\":40}", 40, &uo) == IOT_OK && uo.alarm);
    EXPECT("UT-UC-PLT-01", iot_uc_run_json("{\"uc\":\"plt.export_topics\"}", 27, &uo) == IOT_OK);
    EXPECT("UT-UC-PLT-02", iot_uc_run_json("{\"uc\":\"plt.wifi_backoff\"}", 26, &uo) == IOT_OK);

    iot_command_msg_t ucmd;
    memset(&ucmd, 0, sizeof(ucmd));
    const char *cmdjs = "{\"uc\":\"energy.reset_daily\"}";
    memcpy(ucmd.payload, cmdjs, strlen(cmdjs));
    ucmd.payload_len = (uint16_t)strlen(cmdjs);
    EXPECT("UT-UC-CMD-01", iot_services_handle_command(&ucmd) == IOT_OK);

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
