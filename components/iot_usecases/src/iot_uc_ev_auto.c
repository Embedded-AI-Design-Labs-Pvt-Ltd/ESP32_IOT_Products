/**
 * @file iot_uc_ev_auto.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief EV charger IoT gateway (non-safety) and automotive CAN gateway use cases.
 */
#include "iot_uc_priv.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    char state[16];
    float e_kwh;
    float temp;
    uint32_t session;
    int fault;
} ev_st_t;

static ev_st_t ev = {.state = "Idle"};

typedef struct {
    uint32_t allow_min;
    uint32_t allow_max;
    uint32_t last_id;
    uint32_t speed;
    uint32_t rpm;
    uint32_t coolant;
    uint32_t bus_off;
    uint32_t dtc;
    int accepted;
} auto_st_t;

static auto_st_t can = {.allow_min = 0x100, .allow_max = 0x7FF};

static void ev_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[280];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"chg_state\":\"%s\",\"e_kwh\":%.3f,\"t\":%.1f,"
                   "\"fault\":%d,\"session\":%u,\"safety\":\"external\"}",
                   iot_uc_name(id), ev.state, (double)ev.e_kwh, (double)ev.temp, ev.fault,
                   (unsigned)ev.session);
    iot_uc_out_ok(out, id, js);
    out->alarm = ev.fault != 0;
    out->f[0] = ev.e_kwh;
    out->f[1] = ev.temp;
}

iot_err_t iot_uc_ev_on_sample(int mv, iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.i[0] = mv;
    return iot_uc_ev_sample(&in, out);
}

iot_err_t iot_uc_ev_sample(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int mv = in->i[0];
    ev.e_kwh = (float)mv / 10000.0f;
    ev.temp = 25.0f + ((float)(mv % 200) / 20.0f);
    ev.fault = (ev.temp > 70.0f) ? 1 : 0;
    ev_json(out, IOT_UC_EV_SAMPLE);
    return IOT_OK;
}

iot_err_t iot_uc_ev_start(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    if ((in != NULL) && (in->json != NULL) && (strstr(in->json, "contactor") != NULL)) {
        return iot_uc_ev_reject_contactor(in, out);
    }
    (void)memcpy(ev.state, "Charging", 9);
    if (ev.session == 0U) {
        ev.session = 1U;
    }
    ev_json(out, IOT_UC_EV_START_REQ);
    return IOT_OK;
}

iot_err_t iot_uc_ev_stop(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    (void)memcpy(ev.state, "Idle", 5);
    ev_json(out, IOT_UC_EV_STOP_REQ);
    return IOT_OK;
}

iot_err_t iot_uc_ev_reject_contactor(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_uc_out_err(out, IOT_UC_EV_REJECT_CONTACTOR, IOT_ERR_DENIED);
    (void)snprintf(out->json, sizeof(out->json),
                   "{\"uc\":\"ev.reject_contactor\",\"err\":\"safety_not_on_esp32c3\"}");
    out->err = IOT_ERR_DENIED;
    return IOT_ERR_DENIED;
}

iot_err_t iot_uc_ev_session(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t op = 1;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "session")) {
        (void)iot_uc_json_i32(in->json, "session", &op);
    }
    if (op == 0) {
        ev.session = 0;
        (void)memcpy(ev.state, "Idle", 5);
    } else {
        ev.session++;
    }
    ev_json(out, IOT_UC_EV_SESSION);
    return IOT_OK;
}

iot_err_t iot_uc_ev_fault(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    ev.fault = 1;
    (void)memcpy(ev.state, "Fault", 6);
    ev_json(out, IOT_UC_EV_FAULT_LATCH);
    return IOT_OK;
}

iot_err_t iot_uc_ev_get_state(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    ev_json(out, IOT_UC_EV_GET_STATE);
    return IOT_OK;
}

static void auto_json(iot_uc_out_t *out, iot_uc_id_t id)
{
    char js[280];
    (void)snprintf(js, sizeof(js),
                   "{\"uc\":\"%s\",\"can_id\":%u,\"speed_kph\":%u,\"rpm\":%u,\"coolant\":%u,"
                   "\"accepted\":%d,\"bus_off\":%u,\"dtc\":%u,\"hw\":\"mcp2515\"}",
                   iot_uc_name(id), (unsigned)can.last_id, (unsigned)can.speed, (unsigned)can.rpm,
                   (unsigned)can.coolant, can.accepted, (unsigned)can.bus_off, (unsigned)can.dtc);
    iot_uc_out_ok(out, id, js);
    out->u[0] = can.last_id;
    out->u[1] = can.speed;
}

iot_err_t iot_uc_auto_on_frame(uint32_t can_id, uint32_t speed, uint32_t rpm, uint32_t coolant,
                               iot_uc_out_t *out)
{
    iot_uc_in_t in;
    (void)memset(&in, 0, sizeof(in));
    in.u[0] = can_id;
    in.u[1] = speed;
    in.u[2] = rpm;
    in.u[3] = coolant;
    iot_err_t e = iot_uc_auto_filter(&in, out);
    if (e != IOT_OK) {
        return e;
    }
    return iot_uc_auto_decode(&in, out);
}

iot_err_t iot_uc_auto_filter(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    uint32_t id = in->u[0];
    can.last_id = id;
    can.accepted = (id >= can.allow_min && id <= can.allow_max) ? 1 : 0;
    auto_json(out, IOT_UC_AUTO_FILTER);
    return can.accepted ? IOT_OK : IOT_ERR_DENIED;
}

iot_err_t iot_uc_auto_decode(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    can.speed = in->u[1] % 250U;
    can.rpm = 800U + (in->u[2] % 4000U);
    can.coolant = 70U + (in->u[3] % 40U);
    auto_json(out, IOT_UC_AUTO_DECODE);
    return IOT_OK;
}

iot_err_t iot_uc_auto_reject_tx(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    iot_uc_out_err(out, IOT_UC_AUTO_REJECT_TX, IOT_ERR_DENIED);
    (void)snprintf(out->json, sizeof(out->json),
                   "{\"uc\":\"auto.reject_tx\",\"err\":\"no_cloud_can_inject\"}");
    out->err = IOT_ERR_DENIED;
    return IOT_ERR_DENIED;
}

iot_err_t iot_uc_auto_bus_off(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    can.bus_off++;
    auto_json(out, IOT_UC_AUTO_BUS_OFF);
    out->alarm = true;
    return IOT_OK;
}

iot_err_t iot_uc_auto_dtc(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    int32_t code = 1;
    if ((in->json != NULL) && iot_uc_json_has_key(in->json, "dtc")) {
        (void)iot_uc_json_i32(in->json, "dtc", &code);
    }
    can.dtc = (uint32_t)code;
    auto_json(out, IOT_UC_AUTO_DTC);
    out->alarm = can.dtc != 0U;
    return IOT_OK;
}

iot_err_t iot_uc_auto_get_state(const iot_uc_in_t *in, iot_uc_out_t *out)
{
    (void)in;
    auto_json(out, IOT_UC_AUTO_GET_STATE);
    return IOT_OK;
}
