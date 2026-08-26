/**
 * @file iot_uc_priv.h
 * @brief Internal use-case handlers. Not for application includes.
 */
#ifndef IOT_UC_PRIV_H
#define IOT_UC_PRIV_H

#include "iot_uc.h"

typedef iot_err_t (*iot_uc_fn_t)(const iot_uc_in_t *in, iot_uc_out_t *out);

void iot_uc_out_ok(iot_uc_out_t *out, iot_uc_id_t id, const char *json);
void iot_uc_out_err(iot_uc_out_t *out, iot_uc_id_t id, iot_err_t err);

iot_err_t iot_uc_plt_factory_reset(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_set_sample_ms(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_get_diag(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_provision(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_ota(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_identity(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_set_cloud(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_set_log_level(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_export_topics(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_wifi_backoff(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_cmd_schema(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_mark_ota(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_plt_shutdown(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_energy_measure(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_accum(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_over_power(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_over_voltage(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_under_voltage(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_over_current(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_set_limits(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_calibrate(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_reset_daily(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_pf(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_get_state(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_energy_led_alarm(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_ind_sample(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ind_state(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ind_set_limits(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ind_ack(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ind_hours(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ind_pulse(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_pm_features(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_pm_health(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_pm_trend(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_pm_reset_peak(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_water_meter(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_water_leak(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_water_burst(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_water_valve_open(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_water_valve_close(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_water_tamper(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_water_billing(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_water_set_k(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_water_get_state(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_ev_sample(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ev_start(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ev_stop(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ev_reject_contactor(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ev_session(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ev_fault(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_ev_get_state(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_auto_filter(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_auto_decode(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_auto_reject_tx(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_auto_bus_off(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_auto_dtc(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_auto_get_state(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_trk_beacon(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_trk_motion(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_trk_batt(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_trk_geo(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_trk_sleep(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_agri_soil(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_agri_pump_auto(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_agri_pump_on(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_agri_pump_off(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_agri_schedule(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_agri_inhibit(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_cold_sample(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_cold_excursion(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_cold_door(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_cold_ack(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_cold_datalog(const iot_uc_in_t *in, iot_uc_out_t *out);

iot_err_t iot_uc_robot_uart(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_robot_hb(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_robot_reject(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_robot_estop(const iot_uc_in_t *in, iot_uc_out_t *out);
iot_err_t iot_uc_robot_batt(const iot_uc_in_t *in, iot_uc_out_t *out);

#endif /* IOT_UC_PRIV_H */
