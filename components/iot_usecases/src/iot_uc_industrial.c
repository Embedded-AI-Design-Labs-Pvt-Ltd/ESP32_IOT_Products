/**
 * @file iot_uc_industrial.c
 * @brief Industrial machine monitoring + predictive-maintenance feature use cases.
 */
#include "iot_uc_priv.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef enum { ST_STOP = 0, ST_IDLE, ST_RUN, ST_WARN, ST_FAULT } ind_state_t;

typedef struct {
    float temp;
    float vib;
    float current;
    float pressure;
    float t_lim;
    float vib_lim;
    float i_lim;
    float p_lim;
    float hours;
    uint32_t pulses;
    uint32_t ack_needed;
    ind_state_t st;
} ind_st_t;

static ind_st_t g = {
    .t_lim = 80.0f, .vib_lim = 40.0f, .i_lim = 12.0f, .p_lim = 8.0f, .st = ST_IDLE};

typedef struct {
    float rms;
    float peak;
    float crest;
    float ema;
    int health;
} pm_st_t;

static pm_st_t pm;

static const char *st_name(ind_state_t s)
{
    switch (s) {
    case ST_STOP:
        return "STOP";
    case ST_IDLE:
        return "IDLE";
    case ST_RUN:
        return "RUN";
    case ST_WARN:
        return "WARN";
    case ST_FAULT:
        return "FAULT";
    default:
        return "?";
    }
}

static void ind_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[360];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"t\":%.2f,\"vib\":%.1f,\"i\":%.2f,\"p_bar\":%.2f,"
                   "\"state\":\"%s\",\"hours\":%.2f,\"pulse\":%u}",
                   iot_uc_name(id), (double)g.temp, (double)g.vib, (double)g.current,
                   (double)g.pressure, st_name(g.st), (double)g.hours, (unsigned)g.pulses);
    iot_uc_out_ok(out, id, js);
    out->alarm = (g.st == ST_FAULT) || (g.st == ST_WARN);
    out->f[0] = g.temp;
    out->f[1] = g.vib;
}

static void ind_step_state(void)
{
    if ((g.temp > g.t_lim) || (g.current > g.i_lim) || (g.pressure > g.p_lim)) {
        g.st = ST_FAULT;
        g.ack_needed = 1;
        return;
    }
    if (g.vib > g.vib_lim) {
        g.st = ST_WARN;
        return;
    }
    if (g.st == ST_FAULT) {
        return;
    }
    g.st = (g.current > 0.4f) ? ST_RUN : ST_IDLE;
}

iot_err_t iot_uc_ind_on_sample(float temp, float vib, float current, float pressure, uint32_t pulses,
                               iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.f[0] = temp;
    in.f[1] = vib;
    in.f[2] = current;
    in.f[3] = pressure;
    in.u[0] = pulses;
    return iot_uc_ind_sample(&in, out);
}

iot_err_t iot_uc_ind_sample(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    g.temp = in->f[0];
    g.vib = in->f[1];
    g.current = in->f[2];
    g.pressure = in->f[3];
    if (in->json != NULL) {
        (void)iot_uc_json_f32(in->json, "t", &g.temp);
        (void)iot_uc_json_f32(in->json, "vib", &g.vib);
        (void)iot_uc_json_f32(in->json, "i", &g.current);
        (void)iot_uc_json_f32(in->json, "p_bar", &g.pressure);
    }
    if (in->u[0] > 0U) {
        g.pulses = in->u[0];
    }
    ind_step_state();
    if (g.st == ST_RUN) {
        g.hours += 1.0f / 3600.0f;
    }
    ind_json(out, IOT_UC_IND_SAMPLE);
    return IOT_OK;
}

iot_err_t iot_uc_ind_state(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    ind_step_state();
    ind_json(out, IOT_UC_IND_STATE_MACHINE);
    return IOT_OK;
}

iot_err_t iot_uc_ind_set_limits(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    if (in->json != NULL) {
        (void)iot_uc_json_f32(in->json, "t_lim", &g.t_lim);
        (void)iot_uc_json_f32(in->json, "vib_lim", &g.vib_lim);
        (void)iot_uc_json_f32(in->json, "i_lim", &g.i_lim);
        (void)iot_uc_json_f32(in->json, "p_lim", &g.p_lim);
    }
    ind_json(out, IOT_UC_IND_SET_LIMITS);
    return IOT_OK;
}

iot_err_t iot_uc_ind_ack(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    if (g.st == ST_FAULT) {
        g.st = ST_IDLE;
        g.ack_needed = 0;
    }
    ind_json(out, IOT_UC_IND_ACK_ALARM);
    return IOT_OK;
}

iot_err_t iot_uc_ind_hours(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    float dt_s = in->f[0] > 0.0f ? in->f[0] : 1.0f;
    if (g.st == ST_RUN) {
        g.hours += dt_s / 3600.0f;
    }
    ind_json(out, IOT_UC_IND_HOURS_TICK);
    return IOT_OK;
}

iot_err_t iot_uc_ind_pulse(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    uint32_t add = in->u[0] != 0U ? in->u[0] : 1U;
    g.pulses += add;
    ind_json(out, IOT_UC_IND_PULSE);
    return IOT_OK;
}

static void pm_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[220];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"rms\":%.3f,\"peak\":%.3f,\"crest\":%.3f,\"health\":%d,\"ema\":%.3f}",
                   iot_uc_name(id), (double)pm.rms, (double)pm.peak, (double)pm.crest, pm.health,
                   (double)pm.ema);
    iot_uc_out_ok(out, id, js);
    out->f[0] = pm.rms;
    out->i[0] = pm.health;
    out->alarm = pm.health < 50;
}

iot_err_t iot_uc_pm_on_sample(float accel, iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.f[0] = accel;
    return iot_uc_pm_features(&in, out);
}

iot_err_t iot_uc_pm_features(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    float x = in->f[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "accel")) {
        (void)iot_uc_json_f32(in->json, "accel", &x);
    }
    float ax = fabsf(x);
    pm.rms = (pm.rms * 0.9f) + (ax * 0.1f);
    if (ax > pm.peak) {
        pm.peak = ax;
    }
    pm.crest = (pm.rms > 0.01f) ? (pm.peak / pm.rms) : 0.0f;
    pm.ema = (pm.ema * 0.98f) + (pm.rms * 0.02f);
    return iot_uc_pm_health(in, out);
}

iot_err_t iot_uc_pm_health(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    if (pm.rms < 2.0f) {
        pm.health = 100;
    } else if (pm.rms < 5.0f) {
        pm.health = 70;
    } else if (pm.rms < 8.0f) {
        pm.health = 40;
    } else {
        pm.health = 15;
    }
    pm_json(out, IOT_UC_PM_HEALTH);
    out->id = IOT_UC_PM_FEATURES;
    return IOT_OK;
}

iot_err_t iot_uc_pm_trend(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    pm_json(out, IOT_UC_PM_TREND);
    out->alarm = (pm.rms > (pm.ema * 1.5f + 0.5f));
    return IOT_OK;
}

iot_err_t iot_uc_pm_reset_peak(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    pm.peak = 0.0f;
    pm_json(out, IOT_UC_PM_RESET_PEAK);
    return IOT_OK;
}
