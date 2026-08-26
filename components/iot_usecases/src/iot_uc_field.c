/**
 * @file iot_uc_field.c
 * @brief Asset tracker, agriculture, cold-chain, and robotics gateway use cases.
 */
#include "iot_uc_priv.h"
#include "iot_pwm.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int rssi;
    int batt_mv;
    bool moving;
    int geo_inside;
    uint32_t sleep_s;
} trk_st_t;
static trk_st_t tr = {.rssi = -62, .batt_mv = 3900, .geo_inside = 1, .sleep_s = 30};

typedef struct {
    int moist;
    int pump;
    int thresh;
    int window_min;
    bool inhibit;
} agri_st_t;
static agri_st_t ag = {.thresh = 30, .window_min = 15};

typedef struct {
    float t;
    float rh;
    float t_max;
    float minutes_hot;
    bool door;
    bool alarm;
    bool acked;
} cold_st_t;
static cold_st_t cd = {.t_max = 8.0f, .rh = 55.0f};

typedef struct {
    size_t uart_n;
    uint32_t hb_age_ms;
    int estop;
    int batt_mv;
} rob_st_t;
static rob_st_t rb = {.batt_mv = 22200};

static void tr_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[240];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"rssi_proxy\":%d,\"moving\":%d,\"batt_mv\":%d,"
                   "\"geo_inside\":%d,\"sleep_s\":%u}",
                   iot_uc_name(id), tr.rssi, (int)tr.moving, tr.batt_mv, tr.geo_inside,
                   (unsigned)tr.sleep_s);
    iot_uc_out_ok(out, id, js);
    out->alarm = (tr.batt_mv < 3300) || (tr.geo_inside == 0);
}

iot_err_t iot_uc_trk_on_sample(int rssi, int batt_mv, bool moving, iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.i[0] = rssi;
    in.i[1] = batt_mv;
    in.b[0] = moving;
    (void)iot_uc_trk_beacon(&in, out);
    (void)iot_uc_trk_motion(&in, out);
    return iot_uc_trk_batt(&in, out);
}

iot_err_t iot_uc_trk_beacon(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    if (in->i[0] != 0) {
        tr.rssi = in->i[0];
    }
    tr_json(out, IOT_UC_TRK_BEACON);
    return IOT_OK;
}

iot_err_t iot_uc_trk_motion(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    tr.moving = in->b[0] || ((in->json != NULL) && iot_uc_json_has_key(in->json, "moving"));
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "moving")) {
        int32_t m = 0;
        (void)iot_uc_json_i32(in->json, "moving", &m);
        tr.moving = m != 0;
    }
    tr_json(out, IOT_UC_TRK_MOTION);
    return IOT_OK;
}

iot_err_t iot_uc_trk_batt(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    if (in->i[1] > 0) {
        tr.batt_mv = in->i[1];
    }
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "batt_mv")) {
        int32_t b = 0;
        (void)iot_uc_json_i32(in->json, "batt_mv", &b);
        tr.batt_mv = b;
    }
    tr_json(out, IOT_UC_TRK_BATT_LOW);
    out->alarm = tr.batt_mv < 3300;
    return IOT_OK;
}

iot_err_t iot_uc_trk_geo(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    /* RSSI weaker than -90 treated as outside proxy fence. */
    int lim = -90;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "rssi_lim")) {
        int32_t v = 0;
        (void)iot_uc_json_i32(in->json, "rssi_lim", &v);
        lim = v;
    }
    tr.geo_inside = (tr.rssi >= lim) ? 1 : 0;
    tr_json(out, IOT_UC_TRK_GEOFENCE);
    return IOT_OK;
}

iot_err_t iot_uc_trk_sleep(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t s = (int32_t)tr.sleep_s;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "sleep_s")) {
        (void)iot_uc_json_i32(in->json, "sleep_s", &s);
    } else if (in->i[0] > 0) {
        s = in->i[0];
    }
    if ((s < 5) || (s > 3600)) {
        iot_uc_out_err(out, IOT_UC_TRK_SLEEP, IOT_ERR_INVALID_ARG);
        return IOT_ERR_INVALID_ARG;
    }
    tr.sleep_s = (uint32_t)s;
    tr_json(out, IOT_UC_TRK_SLEEP);
    return IOT_OK;
}

static void ag_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[220];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"moist_pct\":%d,\"pump\":%d,\"thresh\":%d,\"inhibit\":%d}",
                   iot_uc_name(id), ag.moist, ag.pump, ag.thresh, (int)ag.inhibit);
    iot_uc_out_ok(out, id, js);
    out->i[0] = ag.moist;
    out->i[1] = ag.pump;
}

static void pump_hw(int on)
{
    if (ag.inhibit) {
        on = 0;
    }
    ag.pump = on ? 1 : 0;
    (void)iot_pwm_set_duty(on ? 512U : 0U);
}

iot_err_t iot_uc_agri_on_sample(int moist_pct, iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.i[0] = moist_pct;
    (void)iot_uc_agri_soil(&in, out);
    return iot_uc_agri_pump_auto(&in, out);
}

iot_err_t iot_uc_agri_soil(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    ag.moist = in->i[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "moist")) {
        int32_t m = 0;
        (void)iot_uc_json_i32(in->json, "moist", &m);
        ag.moist = m;
    }
    ag_json(out, IOT_UC_AGRI_SOIL);
    return IOT_OK;
}

iot_err_t iot_uc_agri_pump_auto(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    pump_hw(ag.moist < ag.thresh);
    ag_json(out, IOT_UC_AGRI_PUMP_AUTO);
    return IOT_OK;
}

iot_err_t iot_uc_agri_pump_on(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    pump_hw(1);
    ag_json(out, IOT_UC_AGRI_PUMP_ON);
    return ag.inhibit ? IOT_ERR_DENIED : IOT_OK;
}

iot_err_t iot_uc_agri_pump_off(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    pump_hw(0);
    ag_json(out, IOT_UC_AGRI_PUMP_OFF);
    return IOT_OK;
}

iot_err_t iot_uc_agri_schedule(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t w = ag.window_min;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "window_min")) {
        (void)iot_uc_json_i32(in->json, "window_min", &w);
    }
    ag.window_min = w;
    ag_json(out, IOT_UC_AGRI_SCHEDULE);
    return IOT_OK;
}

iot_err_t iot_uc_agri_inhibit(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t inh = 1;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "inhibit")) {
        (void)iot_uc_json_i32(in->json, "inhibit", &inh);
    }
    ag.inhibit = inh != 0;
    if (ag.inhibit) {
        pump_hw(0);
    }
    ag_json(out, IOT_UC_AGRI_INHIBIT);
    return IOT_OK;
}

static void cd_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[260];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"t_c\":%.2f,\"rh\":%.1f,\"door\":%d,\"alarm\":%d,"
                   "\"exc_min\":%.1f,\"acked\":%d}",
                   iot_uc_name(id), (double)cd.t, (double)cd.rh, (int)cd.door, (int)cd.alarm,
                   (double)cd.minutes_hot, (int)cd.acked);
    iot_uc_out_ok(out, id, js);
    out->alarm = cd.alarm;
    out->f[0] = cd.t;
}

iot_err_t iot_uc_cold_on_sample(float t_c, float rh, bool door_open, iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.f[0] = t_c;
    in.f[1] = rh;
    in.b[0] = door_open;
    (void)iot_uc_cold_sample(&in, out);
    (void)iot_uc_cold_door(&in, out);
    return iot_uc_cold_excursion(&in, out);
}

iot_err_t iot_uc_cold_sample(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    cd.t = in->f[0];
    cd.rh = in->f[1] > 0.0f ? in->f[1] : cd.rh;
    if (in->json != NULL) {
        (void)iot_uc_json_f32(in->json, "t_c", &cd.t);
        (void)iot_uc_json_f32(in->json, "rh", &cd.rh);
    }
    cd.alarm = (cd.t > cd.t_max) || cd.door;
    cd_json(out, IOT_UC_COLD_SAMPLE);
    return IOT_OK;
}

iot_err_t iot_uc_cold_excursion(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    if (cd.t > cd.t_max) {
        cd.minutes_hot += 1.0f / 60.0f;
        cd.alarm = true;
        cd.acked = false;
    }
    cd_json(out, IOT_UC_COLD_EXCURSION);
    return IOT_OK;
}

iot_err_t iot_uc_cold_door(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    cd.door = in->b[0];
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "door")) {
        int32_t d = 0;
        (void)iot_uc_json_i32(in->json, "door", &d);
        cd.door = d != 0;
    }
    if (cd.door) {
        cd.alarm = true;
    }
    cd_json(out, IOT_UC_COLD_DOOR);
    return IOT_OK;
}

iot_err_t iot_uc_cold_ack(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    cd.acked = true;
    if (!cd.door && (cd.t <= cd.t_max)) {
        cd.alarm = false;
    }
    cd_json(out, IOT_UC_COLD_ACK);
    return IOT_OK;
}

iot_err_t iot_uc_cold_datalog(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    cd_json(out, IOT_UC_COLD_DATALOG);
    return IOT_OK;
}

static void rb_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[240];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"uart_bytes\":%u,\"hb_ms\":%u,\"estop\":%d,\"batt_mv\":%d}",
                   iot_uc_name(id), (unsigned)rb.uart_n, (unsigned)rb.hb_age_ms, rb.estop, rb.batt_mv);
    iot_uc_out_ok(out, id, js);
    out->alarm = rb.estop != 0;
}

iot_err_t iot_uc_robot_on_uart(size_t n, iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.u[0] = (uint32_t)n;
    (void)iot_uc_robot_uart(&in, out);
    return iot_uc_robot_hb(&in, out);
}

iot_err_t iot_uc_robot_uart(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    rb.uart_n = in->u[0];
    if (rb.uart_n > 0U) {
        rb.hb_age_ms = 0;
    }
    rb_json(out, IOT_UC_ROBOT_UART);
    return IOT_OK;
}

iot_err_t iot_uc_robot_hb(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    if (rb.uart_n == 0U) {
        rb.hb_age_ms += 200;
    }
    rb_json(out, IOT_UC_ROBOT_HEARTBEAT);
    out->alarm = rb.hb_age_ms > 5000U;
    return IOT_OK;
}

iot_err_t iot_uc_robot_reject(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_uc_out_err(out, IOT_UC_ROBOT_REJECT_MOTION, IOT_ERR_DENIED);
    (void)snprintf(out->json, sizeof(out->json),
                   "{\"uc\":\"robot.reject_motion\",\"err\":\"motion_on_robot_mcu_only\"}");
    out->err = IOT_ERR_DENIED;
    return IOT_ERR_DENIED;
}

iot_err_t iot_uc_robot_estop(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t e = 1;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "estop")) {
        (void)iot_uc_json_i32(in->json, "estop", &e);
    }
    rb.estop = e;
    rb_json(out, IOT_UC_ROBOT_ESTOP_REPORT);
    return IOT_OK;
}

iot_err_t iot_uc_robot_batt(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    if (in->i[0] > 0) {
        rb.batt_mv = in->i[0];
    }
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "batt_mv")) {
        int32_t b = 0;
        (void)iot_uc_json_i32(in->json, "batt_mv", &b);
        rb.batt_mv = b;
    }
    rb_json(out, IOT_UC_ROBOT_BATTERY);
    out->alarm = rb.batt_mv < 18000;
    return IOT_OK;
}
