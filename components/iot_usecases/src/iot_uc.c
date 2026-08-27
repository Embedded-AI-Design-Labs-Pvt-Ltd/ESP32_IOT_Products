/**
 * @file iot_uc.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Use-case catalog, JSON helpers, and dispatcher.
 *
 * @syscalls none (handlers may call NVS/OTA/GPIO)
 * @thread command task
 */
#include "iot_uc.h"
#include "iot_uc_priv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    iot_uc_id_t id;
    const char *name;
    const char *summary;
    iot_uc_fn_t fn;
} iot_uc_entry_t;

static const iot_uc_entry_t s_tab[] = {
    {IOT_UC_PLT_FACTORY_RESET, "plt.factory_reset", "Clear NVS provisioned credentials to defaults", iot_uc_plt_factory_reset},
    {IOT_UC_PLT_SET_SAMPLE_MS, "plt.set_sample_ms", "Set telemetry sample period milliseconds", iot_uc_plt_set_sample_ms},
    {IOT_UC_PLT_GET_DIAG, "plt.get_diag", "Return diagnostic snapshot JSON", iot_uc_plt_get_diag},
    {IOT_UC_PLT_PROVISION, "plt.provision", "Apply BLE/MQTT commissioning JSON to NVS", iot_uc_plt_provision},
    {IOT_UC_PLT_OTA, "plt.ota", "Start HTTPS OTA from ota_url", iot_uc_plt_ota},
    {IOT_UC_PLT_IDENTITY, "plt.identity", "Return device id / fw / profile", iot_uc_plt_identity},
    {IOT_UC_PLT_SET_CLOUD, "plt.set_cloud", "Select AWS/Azure/GCP/generic MQTT profile id", iot_uc_plt_set_cloud},
    {IOT_UC_PLT_SET_LOG_LEVEL, "plt.set_log_level", "Set platform log verbosity 1-5", iot_uc_plt_set_log_level},
    {IOT_UC_PLT_EXPORT_TOPICS, "plt.export_topics", "List MQTT topic suffixes", iot_uc_plt_export_topics},
    {IOT_UC_PLT_WIFI_BACKOFF_POLICY, "plt.wifi_backoff", "Document reconnect backoff bounds", iot_uc_plt_wifi_backoff},
    {IOT_UC_PLT_CMD_SCHEMA, "plt.cmd_schema", "Validate inbound command is JSON object", iot_uc_plt_cmd_schema},
    {IOT_UC_PLT_MARK_OTA_VALID, "plt.mark_ota_valid", "Cancel rollback after healthy boot", iot_uc_plt_mark_ota},
    {IOT_UC_PLT_SHUTDOWN, "plt.shutdown", "Request orderly task shutdown bit", iot_uc_plt_shutdown},

    {IOT_UC_ENERGY_MEASURE, "energy.measure", "Sample V/I, compute P/kWh, evaluate alarms", iot_uc_energy_measure},
    {IOT_UC_ENERGY_ACCUM, "energy.accum", "Accumulate energy for dt_ms", iot_uc_energy_accum},
    {IOT_UC_ENERGY_OVER_POWER, "energy.over_power", "Trip over-power threshold", iot_uc_energy_over_power},
    {IOT_UC_ENERGY_OVER_VOLTAGE, "energy.over_voltage", "Trip over-voltage", iot_uc_energy_over_voltage},
    {IOT_UC_ENERGY_UNDER_VOLTAGE, "energy.under_voltage", "Trip under-voltage", iot_uc_energy_under_voltage},
    {IOT_UC_ENERGY_OVER_CURRENT, "energy.over_current", "Trip over-current", iot_uc_energy_over_current},
    {IOT_UC_ENERGY_SET_LIMITS, "energy.set_limits", "Configure V/I/P alarm limits", iot_uc_energy_set_limits},
    {IOT_UC_ENERGY_CALIBRATE, "energy.calibrate", "Set V and I scale factors", iot_uc_energy_calibrate},
    {IOT_UC_ENERGY_RESET_DAILY, "energy.reset_daily", "Zero daily kWh register", iot_uc_energy_reset_daily},
    {IOT_UC_ENERGY_PF, "energy.pf", "Update power-factor estimate", iot_uc_energy_pf},
    {IOT_UC_ENERGY_GET_STATE, "energy.get_state", "Dump energy registers as JSON", iot_uc_energy_get_state},
    {IOT_UC_ENERGY_LED_ALARM, "energy.led_alarm", "Drive alarm LED from last trip", iot_uc_energy_led_alarm},

    {IOT_UC_IND_SAMPLE, "industrial.sample", "Ingest temp/vib/current/pressure", iot_uc_ind_sample},
    {IOT_UC_IND_STATE_MACHINE, "industrial.state", "STOP/IDLE/RUN/WARN/FAULT machine", iot_uc_ind_state},
    {IOT_UC_IND_SET_LIMITS, "industrial.set_limits", "Set industrial alarm limits", iot_uc_ind_set_limits},
    {IOT_UC_IND_ACK_ALARM, "industrial.ack", "Acknowledge latched FAULT to IDLE", iot_uc_ind_ack},
    {IOT_UC_IND_HOURS_TICK, "industrial.hours", "Add operating hours while RUN", iot_uc_ind_hours},
    {IOT_UC_IND_PULSE, "industrial.pulse", "Accumulate shaft/flow pulse count", iot_uc_ind_pulse},

    {IOT_UC_PM_FEATURES, "predictive.features", "RMS/peak/crest vibration features", iot_uc_pm_features},
    {IOT_UC_PM_HEALTH, "predictive.health", "Map RMS to 0-100 health score", iot_uc_pm_health},
    {IOT_UC_PM_TREND, "predictive.trend", "EMA trend of RMS", iot_uc_pm_trend},
    {IOT_UC_PM_RESET_PEAK, "predictive.reset_peak", "Clear peak hold", iot_uc_pm_reset_peak},

    {IOT_UC_WATER_METER, "water.meter", "Convert pulses to flow and totalizers", iot_uc_water_meter},
    {IOT_UC_WATER_LEAK, "water.leak", "Leak when flow with valve closed", iot_uc_water_leak},
    {IOT_UC_WATER_BURST, "water.burst", "Burst if flow exceeds burst_lpm", iot_uc_water_burst},
    {IOT_UC_WATER_VALVE_OPEN, "water.valve_open", "Open remote valve GPIO", iot_uc_water_valve_open},
    {IOT_UC_WATER_VALVE_CLOSE, "water.valve_close", "Close remote valve GPIO", iot_uc_water_valve_close},
    {IOT_UC_WATER_TAMPER, "water.tamper", "Latch magnetic/cover tamper", iot_uc_water_tamper},
    {IOT_UC_WATER_BILLING_FREEZE, "water.billing_freeze", "Freeze billing register", iot_uc_water_billing},
    {IOT_UC_WATER_SET_K, "water.set_k", "Set pulses per litre k-factor", iot_uc_water_set_k},
    {IOT_UC_WATER_GET_STATE, "water.get_state", "Dump meter registers", iot_uc_water_get_state},

    {IOT_UC_EV_SAMPLE, "ev.sample", "Gateway status/energy/temp (not safety)", iot_uc_ev_sample},
    {IOT_UC_EV_START_REQ, "ev.start", "Request start — forwarded as status only", iot_uc_ev_start},
    {IOT_UC_EV_STOP_REQ, "ev.stop", "Request idle/stop status", iot_uc_ev_stop},
    {IOT_UC_EV_REJECT_CONTACTOR, "ev.reject_contactor", "Always deny contactor/safety commands", iot_uc_ev_reject_contactor},
    {IOT_UC_EV_SESSION, "ev.session", "Allocate/clear charging session id", iot_uc_ev_session},
    {IOT_UC_EV_FAULT_LATCH, "ev.fault", "Latch over-temp display fault", iot_uc_ev_fault},
    {IOT_UC_EV_GET_STATE, "ev.get_state", "Dump EV gateway state", iot_uc_ev_get_state},

    {IOT_UC_AUTO_FILTER, "auto.filter", "Accept 11-bit IDs in allow-list", iot_uc_auto_filter},
    {IOT_UC_AUTO_DECODE, "auto.decode", "Decode speed/rpm/coolant signals", iot_uc_auto_decode},
    {IOT_UC_AUTO_REJECT_TX, "auto.reject_tx", "Deny cloud CAN inject", iot_uc_auto_reject_tx},
    {IOT_UC_AUTO_BUS_OFF, "auto.bus_off", "Increment bus-off counter", iot_uc_auto_bus_off},
    {IOT_UC_AUTO_DTC, "auto.dtc", "Latch diagnostic trouble code flag", iot_uc_auto_dtc},
    {IOT_UC_AUTO_GET_STATE, "auto.get_state", "Dump CAN gateway state", iot_uc_auto_get_state},

    {IOT_UC_TRK_BEACON, "tracker.beacon", "Update last-seen / RSSI beacon", iot_uc_trk_beacon},
    {IOT_UC_TRK_MOTION, "tracker.motion", "Set moving flag from IMU/GPIO", iot_uc_trk_motion},
    {IOT_UC_TRK_BATT_LOW, "tracker.batt_low", "Battery-low alarm", iot_uc_trk_batt},
    {IOT_UC_TRK_GEOFENCE, "tracker.geofence", "RSSI proxy geofence", iot_uc_trk_geo},
    {IOT_UC_TRK_SLEEP, "tracker.sleep", "Record sleep-interval hint", iot_uc_trk_sleep},

    {IOT_UC_AGRI_SOIL, "agri.soil", "Soil moisture percent", iot_uc_agri_soil},
    {IOT_UC_AGRI_PUMP_AUTO, "agri.pump_auto", "Auto irrigate below threshold", iot_uc_agri_pump_auto},
    {IOT_UC_AGRI_PUMP_ON, "agri.pump_on", "Manual pump on", iot_uc_agri_pump_on},
    {IOT_UC_AGRI_PUMP_OFF, "agri.pump_off", "Manual pump off", iot_uc_agri_pump_off},
    {IOT_UC_AGRI_SCHEDULE, "agri.schedule", "Set irrigation window minutes", iot_uc_agri_schedule},
    {IOT_UC_AGRI_INHIBIT, "agri.inhibit", "Inhibit pump if tank empty", iot_uc_agri_inhibit},

    {IOT_UC_COLD_SAMPLE, "cold.sample", "Temperature/RH sample", iot_uc_cold_sample},
    {IOT_UC_COLD_EXCURSION, "cold.excursion", "Time-above-limit excursion", iot_uc_cold_excursion},
    {IOT_UC_COLD_DOOR, "cold.door", "Door-open event", iot_uc_cold_door},
    {IOT_UC_COLD_ACK, "cold.ack", "Acknowledge cold-chain alarm", iot_uc_cold_ack},
    {IOT_UC_COLD_DATALOG, "cold.datalog", "Push last N excursion minutes", iot_uc_cold_datalog},

    {IOT_UC_ROBOT_UART, "robot.uart", "Ingest robot MCU UART bytes", iot_uc_robot_uart},
    {IOT_UC_ROBOT_HEARTBEAT, "robot.heartbeat", "Robot MCU heartbeat age", iot_uc_robot_hb},
    {IOT_UC_ROBOT_REJECT_MOTION, "robot.reject_motion", "Deny cloud motion/estop drive", iot_uc_robot_reject},
    {IOT_UC_ROBOT_ESTOP_REPORT, "robot.estop_report", "Report E-stop flag from MCU", iot_uc_robot_estop},
    {IOT_UC_ROBOT_BATTERY, "robot.battery", "Robot battery millivolts", iot_uc_robot_batt},
};

static bool s_init;

void iot_uc_out_ok(iot_uc_out_t *out, iot_uc_id_t id, const char *json)
{
    if (out == NULL) {
        return;
    }
    out->err = IOT_OK;
    out->id = id;
    if (json != NULL) {
        (void)snprintf(out->json, sizeof(out->json), "%s", json);
    }
}

void iot_uc_out_err(iot_uc_out_t *out, iot_uc_id_t id, iot_err_t err)
{
    if (out == NULL) {
        return;
    }
    out->err = err;
    out->id = id;
    (void)snprintf(out->json, sizeof(out->json), "{\"uc\":%d,\"err\":%d}", (int)id, (int)err);
}

static const iot_uc_entry_t *find_id(iot_uc_id_t id)
{
    for (size_t i = 0; i < (sizeof(s_tab) / sizeof(s_tab[0])); i++) {
        if (s_tab[i].id == id) {
            return &s_tab[i];
        }
    }
    return NULL;
}

iot_err_t iot_uc_init(void)
{
    s_init = true;
    return IOT_OK;
}

size_t iot_uc_count(void)
{
    return sizeof(s_tab) / sizeof(s_tab[0]);
}

const char *iot_uc_name(iot_uc_id_t id)
{
    const iot_uc_entry_t *e = find_id(id);
    return e != NULL ? e->name : "invalid";
}

const char *iot_uc_summary(iot_uc_id_t id)
{
    const iot_uc_entry_t *e = find_id(id);
    return e != NULL ? e->summary : "";
}

iot_uc_id_t iot_uc_id_from_name(const char *name)
{
    if (name == NULL) {
        return IOT_UC_INVALID;
    }
    for (size_t i = 0; i < (sizeof(s_tab) / sizeof(s_tab[0])); i++) {
        if (strcmp(s_tab[i].name, name) == 0) {
            return s_tab[i].id;
        }
    }
    return IOT_UC_INVALID;
}

bool iot_uc_json_has_key(const char *json, const char *key)
{
    if ((json == NULL) || (key == NULL)) {
        return false;
    }
    char pat[48];
    (void)snprintf(pat, sizeof(pat), "\"%s\"", key);
    return strstr(json, pat) != NULL;
}

bool iot_uc_json_str(const char *json, const char *key, char *out, size_t out_len)
{
    if ((json == NULL) || (key == NULL) || (out == NULL) || (out_len == 0U)) {
        return false;
    }
    char pat[48];
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
    while (*p == ' ' || *p == '\t') {
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

bool iot_uc_json_f32(const char *json, const char *key, float *out)
{
    if ((json == NULL) || (key == NULL) || (out == NULL)) {
        return false;
    }
    char pat[48];
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
    *out = (float)atof(p);
    return true;
}

bool iot_uc_json_i32(const char *json, const char *key, int32_t *out)
{
    float f = 0.0f;
    if (!iot_uc_json_f32(json, key, &f)) {
        return false;
    }
    *out = (int32_t)f;
    return true;
}

iot_err_t iot_uc_run(iot_uc_id_t id, const iot_uc_in_t *in, iot_uc_out_t *out)
{
    if (out == NULL) {
        return IOT_ERR_INVALID_ARG;
    }
    if (!s_init) {
        (void)iot_uc_init();
    }
    (void)memset(out, 0, sizeof(*out));
    const iot_uc_entry_t *e = find_id(id);
    if ((e == NULL) || (e->fn == NULL)) {
        iot_uc_out_err(out, id, IOT_ERR_NOT_FOUND);
        return IOT_ERR_NOT_FOUND;
    }
    iot_uc_in_t dummy;
    if (in == NULL) {
        (void)memset(&dummy, 0, sizeof(dummy));
        in = &dummy;
    }
    return e->fn(in, out);
}

iot_err_t iot_uc_run_json(const char *json, size_t len, iot_uc_out_t *out)
{
    if ((json == NULL) || (len == 0U) || (out == NULL)) {
        return IOT_ERR_INVALID_ARG;
    }
    char name[48];
    if (!iot_uc_json_str(json, "uc", name, sizeof(name))) {
        if (out != NULL) {
            iot_uc_out_err(out, IOT_UC_INVALID, IOT_ERR_PROTOCOL);
        }
        return IOT_ERR_PROTOCOL;
    }
    iot_uc_id_t id = iot_uc_id_from_name(name);
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.json = json;
    in.json_len = len;
    (void)iot_uc_json_f32(json, "f0", &in.f[0]);
    (void)iot_uc_json_f32(json, "f1", &in.f[1]);
    (void)iot_uc_json_f32(json, "f2", &in.f[2]);
    (void)iot_uc_json_i32(json, "i0", &in.i[0]);
    (void)iot_uc_json_str(json, "s0", in.s0, sizeof(in.s0));
    (void)iot_uc_json_str(json, "ota_url", in.s1, sizeof(in.s1));
    return iot_uc_run(id, &in, out);
}

iot_err_t iot_uc_dispatch_command(const iot_command_msg_t *cmd, iot_uc_out_t *out)
{
    if ((cmd == NULL) || (cmd->payload_len == 0U)) {
        return IOT_ERR_INVALID_ARG;
    }
    return iot_uc_run_json((const char *)cmd->payload, cmd->payload_len, out);
}
