/**
 * @file iot_uc_energy.c
 * @brief Smart energy monitor use cases — measurement, limits, calibration, daily kWh.
 *
 * External: voltage divider + CT/shunt. Not a certified revenue meter.
 */
#include "iot_uc_priv.h"
#include "iot_gpio.h"
#include "iot_board.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    float v_scale;
    float i_scale;
    float vrms;
    float irms;
    float p;
    float pf;
    float kwh;
    float kwh_day;
    float over_w;
    float over_v;
    float under_v;
    float over_i;
    uint32_t trips;
    bool alarm;
} energy_st_t;

static energy_st_t s = {
    .v_scale = 1.0f,
    .i_scale = 1.0f,
    .vrms = 230.0f,
    .irms = 0.4f,
    .pf = 0.95f,
    .over_w = 4000.0f,
    .over_v = 253.0f,
    .under_v = 207.0f,
    .over_i = 16.0f,
};

static void energy_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[320];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"v\":%.2f,\"i\":%.3f,\"p\":%.1f,\"pf\":%.3f,"
                   "\"e_kwh\":%.5f,\"e_day\":%.5f,\"alarm\":%d,\"trips\":%u}",
                   iot_uc_name(id), (double)s.vrms, (double)s.irms, (double)s.p, (double)s.pf,
                   (double)s.kwh, (double)s.kwh_day, (int)s.alarm, (unsigned)s.trips);
    iot_uc_out_ok(out, id, js);
    out->f[0] = s.vrms;
    out->f[1] = s.irms;
    out->f[2] = s.p;
    out->f[3] = s.kwh;
    out->alarm = s.alarm;
    out->u[0] = s.trips;
}

static void eval_alarms(void)
{
    s.alarm = (s.p > s.over_w) || (s.vrms > s.over_v) || (s.vrms < s.under_v) || (s.irms > s.over_i);
    if (s.alarm) {
        s.trips++;
    }
}

bool iot_uc_energy_alarm(void)
{
    return s.alarm;
}

iot_err_t iot_uc_energy_on_adc(int mv, iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    /* Board mapping: millivolts → scaled RMS (replace with real CT math after calibration). */
    in.f[0] = (80.0f + ((float)mv * 0.05f)) * s.v_scale;
    in.f[1] = (0.15f + ((float)(mv % 800) / 2000.0f)) * s.i_scale;
    in.f[2] = 1000.0f; /* assume 1 s equivalent if called from 1 Hz path; accum uses dt */
    return iot_uc_energy_measure(&in, out);
}

iot_err_t iot_uc_energy_measure(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    float v = in->f[0];
    float i = in->f[1];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "v")) {
        (void)iot_uc_json_f32(in->json, "v", &v);
        (void)iot_uc_json_f32(in->json, "i", &i);
    }
    if (v > 0.0f) {
        s.vrms = v * s.v_scale;
    }
    if (i >= 0.0f) {
        s.irms = i * s.i_scale;
    }
    s.p = s.vrms * s.irms * s.pf;
    s.kwh += s.p / 3600000.0f;
    s.kwh_day += s.p / 3600000.0f;
    eval_alarms();
    (void)iot_uc_energy_led_alarm(in, out);
    energy_json(out, IOT_UC_ENERGY_MEASURE);
    return IOT_OK;
}

iot_err_t iot_uc_energy_accum(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    float dt_ms = in->f[0] > 0.0f ? in->f[0] : 1000.0f;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "dt_ms")) {
        (void)iot_uc_json_f32(in->json, "dt_ms", &dt_ms);
    }
    s.kwh += s.p * (dt_ms / 3600000000.0f) * 1000.0f;
    energy_json(out, IOT_UC_ENERGY_ACCUM);
    return IOT_OK;
}

iot_err_t iot_uc_energy_over_power(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    s.alarm = s.p > s.over_w;
    if (s.alarm) {
        s.trips++;
    }
    energy_json(out, IOT_UC_ENERGY_OVER_POWER);
    return s.alarm ? IOT_OK : IOT_OK;
}

iot_err_t iot_uc_energy_over_voltage(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    s.alarm = s.vrms > s.over_v;
    energy_json(out, IOT_UC_ENERGY_OVER_VOLTAGE);
    return IOT_OK;
}

iot_err_t iot_uc_energy_under_voltage(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    s.alarm = s.vrms < s.under_v;
    energy_json(out, IOT_UC_ENERGY_UNDER_VOLTAGE);
    return IOT_OK;
}

iot_err_t iot_uc_energy_over_current(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    s.alarm = s.irms > s.over_i;
    energy_json(out, IOT_UC_ENERGY_OVER_CURRENT);
    return IOT_OK;
}

iot_err_t iot_uc_energy_set_limits(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    if (in->json != NULL) {
        (void)iot_uc_json_f32(in->json, "over_w", &s.over_w);
        (void)iot_uc_json_f32(in->json, "over_v", &s.over_v);
        (void)iot_uc_json_f32(in->json, "under_v", &s.under_v);
        (void)iot_uc_json_f32(in->json, "over_i", &s.over_i);
    } else {
        if (in->f[0] > 0.0f) {
            s.over_w = in->f[0];
        }
        if (in->f[1] > 0.0f) {
            s.over_v = in->f[1];
        }
        if (in->f[2] > 0.0f) {
            s.under_v = in->f[2];
        }
        if (in->f[3] > 0.0f) {
            s.over_i = in->f[3];
        }
    }
    energy_json(out, IOT_UC_ENERGY_SET_LIMITS);
    return IOT_OK;
}

iot_err_t iot_uc_energy_calibrate(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    if (in->json != NULL) {
        (void)iot_uc_json_f32(in->json, "v_scale", &s.v_scale);
        (void)iot_uc_json_f32(in->json, "i_scale", &s.i_scale);
    } else {
        if (in->f[0] > 0.0f) {
            s.v_scale = in->f[0];
        }
        if (in->f[1] > 0.0f) {
            s.i_scale = in->f[1];
        }
    }
    if ((s.v_scale < 0.1f) || (s.v_scale > 10.0f) || (s.i_scale < 0.1f) || (s.i_scale > 10.0f)) {
        iot_uc_out_err(out, IOT_UC_ENERGY_CALIBRATE, IOT_ERR_INVALID_ARG);
        return IOT_ERR_INVALID_ARG;
    }
    energy_json(out, IOT_UC_ENERGY_CALIBRATE);
    return IOT_OK;
}

iot_err_t iot_uc_energy_reset_daily(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    s.kwh_day = 0.0f;
    energy_json(out, IOT_UC_ENERGY_RESET_DAILY);
    return IOT_OK;
}

iot_err_t iot_uc_energy_pf(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    float pf = in->f[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "pf")) {
        (void)iot_uc_json_f32(in->json, "pf", &pf);
    }
    if ((pf < 0.5f) || (pf > 1.0f)) {
        iot_uc_out_err(out, IOT_UC_ENERGY_PF, IOT_ERR_INVALID_ARG);
        return IOT_ERR_INVALID_ARG;
    }
    s.pf = pf;
    s.p = s.vrms * s.irms * s.pf;
    energy_json(out, IOT_UC_ENERGY_PF);
    return IOT_OK;
}

iot_err_t iot_uc_energy_get_state(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    energy_json(out, IOT_UC_ENERGY_GET_STATE);
    return IOT_OK;
}

iot_err_t iot_uc_energy_led_alarm(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    (void)iot_gpio_write(IOT_PIN_LED, s.alarm);
    energy_json(out, IOT_UC_ENERGY_LED_ALARM);
    return IOT_OK;
}
