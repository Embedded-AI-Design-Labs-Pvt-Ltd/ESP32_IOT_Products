/**
 * @file iot_uc.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Complete use-case catalog for the ESP32-C3 IoT platform.
 *
 * Cloud/BLE command format:
 *   {"uc":"energy.set_limits","over_w":3500,"over_v":253,"under_v":207,"over_i":16}
 *
 * @thread command task (and host tests). Not ISR-safe.
 */
#ifndef IOT_UC_H
#define IOT_UC_H

#include "iot_err.h"
#include "iot_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IOT_UC_INVALID = 0,

    /* ---- Platform ---- */
    IOT_UC_PLT_FACTORY_RESET,
    IOT_UC_PLT_SET_SAMPLE_MS,
    IOT_UC_PLT_GET_DIAG,
    IOT_UC_PLT_PROVISION,
    IOT_UC_PLT_OTA,
    IOT_UC_PLT_IDENTITY,
    IOT_UC_PLT_SET_CLOUD,
    IOT_UC_PLT_SET_LOG_LEVEL,
    IOT_UC_PLT_EXPORT_TOPICS,
    IOT_UC_PLT_WIFI_BACKOFF_POLICY,
    IOT_UC_PLT_CMD_SCHEMA,
    IOT_UC_PLT_MARK_OTA_VALID,
    IOT_UC_PLT_SHUTDOWN,

    /* ---- Smart energy ---- */
    IOT_UC_ENERGY_MEASURE,
    IOT_UC_ENERGY_ACCUM,
    IOT_UC_ENERGY_OVER_POWER,
    IOT_UC_ENERGY_OVER_VOLTAGE,
    IOT_UC_ENERGY_UNDER_VOLTAGE,
    IOT_UC_ENERGY_OVER_CURRENT,
    IOT_UC_ENERGY_SET_LIMITS,
    IOT_UC_ENERGY_CALIBRATE,
    IOT_UC_ENERGY_RESET_DAILY,
    IOT_UC_ENERGY_PF,
    IOT_UC_ENERGY_GET_STATE,
    IOT_UC_ENERGY_LED_ALARM,

    /* ---- Industrial ---- */
    IOT_UC_IND_SAMPLE,
    IOT_UC_IND_STATE_MACHINE,
    IOT_UC_IND_SET_LIMITS,
    IOT_UC_IND_ACK_ALARM,
    IOT_UC_IND_HOURS_TICK,
    IOT_UC_IND_PULSE,

    /* ---- Predictive ---- */
    IOT_UC_PM_FEATURES,
    IOT_UC_PM_HEALTH,
    IOT_UC_PM_TREND,
    IOT_UC_PM_RESET_PEAK,

    /* ---- Water ---- */
    IOT_UC_WATER_METER,
    IOT_UC_WATER_LEAK,
    IOT_UC_WATER_BURST,
    IOT_UC_WATER_VALVE_OPEN,
    IOT_UC_WATER_VALVE_CLOSE,
    IOT_UC_WATER_TAMPER,
    IOT_UC_WATER_BILLING_FREEZE,
    IOT_UC_WATER_SET_K,
    IOT_UC_WATER_GET_STATE,

    /* ---- EV charger gateway (non-safety) ---- */
    IOT_UC_EV_SAMPLE,
    IOT_UC_EV_START_REQ,
    IOT_UC_EV_STOP_REQ,
    IOT_UC_EV_REJECT_CONTACTOR,
    IOT_UC_EV_SESSION,
    IOT_UC_EV_FAULT_LATCH,
    IOT_UC_EV_GET_STATE,

    /* ---- Automotive CAN gateway ---- */
    IOT_UC_AUTO_FILTER,
    IOT_UC_AUTO_DECODE,
    IOT_UC_AUTO_REJECT_TX,
    IOT_UC_AUTO_BUS_OFF,
    IOT_UC_AUTO_DTC,
    IOT_UC_AUTO_GET_STATE,

    /* ---- BLE asset tracker ---- */
    IOT_UC_TRK_BEACON,
    IOT_UC_TRK_MOTION,
    IOT_UC_TRK_BATT_LOW,
    IOT_UC_TRK_GEOFENCE,
    IOT_UC_TRK_SLEEP,

    /* ---- Agriculture ---- */
    IOT_UC_AGRI_SOIL,
    IOT_UC_AGRI_PUMP_AUTO,
    IOT_UC_AGRI_PUMP_ON,
    IOT_UC_AGRI_PUMP_OFF,
    IOT_UC_AGRI_SCHEDULE,
    IOT_UC_AGRI_INHIBIT,

    /* ---- Cold-chain ---- */
    IOT_UC_COLD_SAMPLE,
    IOT_UC_COLD_EXCURSION,
    IOT_UC_COLD_DOOR,
    IOT_UC_COLD_ACK,
    IOT_UC_COLD_DATALOG,

    /* ---- Robotics gateway ---- */
    IOT_UC_ROBOT_UART,
    IOT_UC_ROBOT_HEARTBEAT,
    IOT_UC_ROBOT_REJECT_MOTION,
    IOT_UC_ROBOT_ESTOP_REPORT,
    IOT_UC_ROBOT_BATTERY,

    IOT_UC_COUNT
} iot_uc_id_t;

typedef struct {
    const char *json;
    size_t json_len;
    float f[8];
    int32_t i[8];
    uint32_t u[8];
    bool b[8];
    char s0[64];
    char s1[160];
} iot_uc_in_t;

typedef struct {
    iot_err_t err;
    iot_uc_id_t id;
    bool alarm;
    float f[8];
    int32_t i[8];
    uint32_t u[8];
    char json[IOT_JSON_TELEMETRY_LEN];
} iot_uc_out_t;

iot_err_t iot_uc_init(void);
size_t iot_uc_count(void);
iot_uc_id_t iot_uc_id_from_name(const char *name);
const char *iot_uc_name(iot_uc_id_t id);
const char *iot_uc_summary(iot_uc_id_t id);

iot_err_t iot_uc_run(iot_uc_id_t id, const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_run_json(const char *json, size_t len, iot_uc_out_t *out);
iot_err_t iot_uc_dispatch_command(const iot_command_msg_t *cmd, iot_uc_out_t *out);

/** C++ catalog size (UseCaseEngine::all). */
size_t iot_uc_cpp_catalog_size(void);
iot_err_t iot_uc_cpp_run_json(const char *json, iot_uc_out_t *out);

bool iot_uc_json_has_key(const char *json, const char *key);
bool iot_uc_json_str(const char *json, const char *key, char *out, size_t out_len);
bool iot_uc_json_f32(const char *json, const char *key, float *out);
bool iot_uc_json_i32(const char *json, const char *key, int32_t *out);

/** Sensor-path helpers used by product profiles (HAL already sampled). */
iot_err_t iot_uc_energy_on_adc(int mv, iot_uc_out_t *out);
iot_err_t iot_uc_ind_on_sample(float temp, float vib, float current, float pressure, uint32_t pulses,
                               iot_uc_out_t *out);
iot_err_t iot_uc_pm_on_sample(float accel, iot_uc_out_t *out);
iot_err_t iot_uc_water_on_sample(uint32_t pulses, int tank_mv, iot_uc_out_t *out);
iot_err_t iot_uc_ev_on_sample(int mv, iot_uc_out_t *out);
iot_err_t iot_uc_auto_on_frame(uint32_t can_id, uint32_t speed, uint32_t rpm, uint32_t coolant,
                               iot_uc_out_t *out);
iot_err_t iot_uc_trk_on_sample(int rssi, int batt_mv, bool moving, iot_uc_out_t *out);
iot_err_t iot_uc_agri_on_sample(int moist_pct, iot_uc_out_t *out);
iot_err_t iot_uc_cold_on_sample(float t_c, float rh, bool door_open, iot_uc_out_t *out);
iot_err_t iot_uc_robot_on_uart(size_t n, iot_uc_out_t *out);

bool iot_uc_water_valve_is_open(void);
bool iot_uc_energy_alarm(void);

#ifdef __cplusplus
}
#endif

#endif /* IOT_UC_H */
