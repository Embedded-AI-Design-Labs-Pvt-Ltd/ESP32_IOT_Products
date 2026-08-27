/**
 * @file iot_uc_water.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Smart water meter use cases: metering, leak, burst, valve, tamper, billing freeze.
 */
#include "iot_uc_priv.h"
#include "iot_gpio.h"
#include "iot_board.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    float k_ppl; /* pulses per litre */
    float flow_lpm;
    float liters;
    float billing;
    float tank_pct;
    float burst_lpm;
    uint32_t last_pulses;
    bool valve_open;
    bool leak;
    bool burst;
    bool tamper;
    bool frozen;
} water_st_t;

static water_st_t w = {.k_ppl = 2.0f, .burst_lpm = 40.0f, .valve_open = true};

bool iot_uc_water_valve_is_open(void)
{
    return w.valve_open;
}

static void water_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[360];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"flow_lpm\":%.2f,\"liters\":%.3f,\"billing\":%.3f,"
                   "\"tank_pct\":%.1f,\"valve\":%d,\"leak\":%d,\"burst\":%d,\"tamper\":%d,\"frozen\":%d}",
                   iot_uc_name(id), (double)w.flow_lpm, (double)w.liters, (double)w.billing,
                   (double)w.tank_pct, (int)w.valve_open, (int)w.leak, (int)w.burst, (int)w.tamper,
                   (int)w.frozen);
    iot_uc_out_ok(out, id, js);
    out->f[0] = w.flow_lpm;
    out->f[1] = w.liters;
    out->alarm = w.leak || w.burst || w.tamper;
}

static void apply_valve(bool open)
{
    w.valve_open = open;
    (void)iot_gpio_write(IOT_PIN_PWM_OUT, open);
}

iot_err_t iot_uc_water_on_sample(uint32_t pulses, int tank_mv, iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.u[0] = pulses;
    in.i[0] = tank_mv;
    return iot_uc_water_meter(&in, out);
}

iot_err_t iot_uc_water_meter(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    uint32_t pulses = in->u[0];
    int tank_mv = in->i[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "pulses")) {
        int32_t p = 0;
        (void)iot_uc_json_i32(in->json, "pulses", &p);
        pulses = (uint32_t)p;
    }
    uint32_t d = (pulses >= w.last_pulses) ? (pulses - w.last_pulses) : 0U;
    w.last_pulses = pulses;
    float litres_delta = (w.k_ppl > 0.01f) ? ((float)d / w.k_ppl) : 0.0f;
    w.flow_lpm = litres_delta * 60.0f; /* assuming ~1 s sample */
    if (!w.frozen) {
        w.liters += litres_delta;
        w.billing += litres_delta;
    }
    if (tank_mv > 0) {
        w.tank_pct = ((float)tank_mv / 3300.0f) * 100.0f;
        if (w.tank_pct > 100.0f) {
            w.tank_pct = 100.0f;
        }
    }
    (void)iot_uc_water_leak(in, out);
    (void)iot_uc_water_burst(in, out);
    water_json(out, IOT_UC_WATER_METER);
    return IOT_OK;
}

iot_err_t iot_uc_water_leak(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    w.leak = (w.flow_lpm > 0.1f) && !w.valve_open;
    water_json(out, IOT_UC_WATER_LEAK);
    return IOT_OK;
}

iot_err_t iot_uc_water_burst(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    w.burst = w.flow_lpm > w.burst_lpm;
    if (w.burst) {
        apply_valve(false);
    }
    water_json(out, IOT_UC_WATER_BURST);
    return IOT_OK;
}

iot_err_t iot_uc_water_valve_open(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    if (w.tamper) {
        iot_uc_out_err(out, IOT_UC_WATER_VALVE_OPEN, IOT_ERR_DENIED);
        return IOT_ERR_DENIED;
    }
    apply_valve(true);
    water_json(out, IOT_UC_WATER_VALVE_OPEN);
    return IOT_OK;
}

iot_err_t iot_uc_water_valve_close(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    apply_valve(false);
    water_json(out, IOT_UC_WATER_VALVE_CLOSE);
    return IOT_OK;
}

iot_err_t iot_uc_water_tamper(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    w.tamper = true;
    apply_valve(false);
    water_json(out, IOT_UC_WATER_TAMPER);
    out->alarm = true;
    return IOT_OK;
}

iot_err_t iot_uc_water_billing(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t freeze = 1;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "freeze")) {
        (void)iot_uc_json_i32(in->json, "freeze", &freeze);
    }
    w.frozen = freeze != 0;
    water_json(out, IOT_UC_WATER_BILLING_FREEZE);
    return IOT_OK;
}

iot_err_t iot_uc_water_set_k(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    float k = in->f[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "k")) {
        (void)iot_uc_json_f32(in->json, "k", &k);
    }
    if ((k < 0.1f) || (k > 1000.0f)) {
        iot_uc_out_err(out, IOT_UC_WATER_SET_K, IOT_ERR_INVALID_ARG);
        return IOT_ERR_INVALID_ARG;
    }
    w.k_ppl = k;
    water_json(out, IOT_UC_WATER_SET_K);
    return IOT_OK;
}

iot_err_t iot_uc_water_get_state(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    water_json(out, IOT_UC_WATER_GET_STATE);
    return IOT_OK;
}
