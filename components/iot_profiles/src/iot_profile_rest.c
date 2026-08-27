/**
 * @file iot_profile_rest.c
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Tracker, agriculture, cold-chain, robotics — use-case ingest + command dispatch.
 */
#include "iot_profile.h"
#include "iot_adc.h"
#include "iot_i2c.h"
#include "iot_uart.h"
#include "iot_pwm.h"
#include "iot_board.h"
#include "iot_services.h"
#include "iot_log.h"
#include "iot_uc.h"

#include <stdio.h>
#include <string.h>

iot_err_t iot_profile_tracker_init(void)
{
    IOT_LOGI("tracker", "BLE asset tracker — optional GNSS module on UART1");
    return IOT_OK;
}

iot_err_t iot_profile_tracker_sensor(iot_telemetry_msg_t *out)
{
    iot_uc_out_t uco;
    (void)iot_uc_trk_on_sample(-62, 3900, false, &uco);
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_tracker_cmd(const iot_command_msg_t *cmd)
{
    return iot_profile_dispatch_cmd(cmd);
}

iot_err_t iot_profile_agri_init(void)
{
    iot_i2c_cfg_t i2c = {.sda = IOT_PIN_I2C_SDA, .scl = IOT_PIN_I2C_SCL, .hz = 100000};
    (void)iot_i2c_init(&i2c);
    (void)iot_adc_init(IOT_PIN_ADC_V);
    (void)iot_pwm_init(IOT_PIN_PWM_OUT, 1000, 10);
    IOT_LOGI("agri", "soil moisture (ADC) + pump PWM — sensors required");
    return IOT_OK;
}

iot_err_t iot_profile_agri_sensor(iot_telemetry_msg_t *out)
{
    int mv = 0;
    iot_uc_out_t uco;
    (void)iot_adc_read_mv(IOT_PIN_ADC_V, &mv);
    int moist = (int)((3300 - mv) * 100 / 3300);
    (void)iot_uc_agri_on_sample(moist, &uco);
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_agri_cmd(const iot_command_msg_t *cmd)
{
    return iot_profile_dispatch_cmd(cmd);
}

iot_err_t iot_profile_coldchain_init(void)
{
    iot_i2c_cfg_t i2c = {.sda = IOT_PIN_I2C_SDA, .scl = IOT_PIN_I2C_SCL, .hz = 100000};
    (void)iot_i2c_init(&i2c);
    IOT_LOGI("cold", "I2C temperature/humidity (SHT3x/TMP117) required");
    return IOT_OK;
}

iot_err_t iot_profile_coldchain_sensor(iot_telemetry_msg_t *out)
{
    uint8_t raw[2] = {0};
    iot_uc_out_t uco;
    (void)iot_i2c_read(0x44, raw, 2, 40U);
    float t = 4.0f + ((float)raw[0] / 50.0f);
    (void)iot_uc_cold_on_sample(t, 55.0f, false, &uco);
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_coldchain_cmd(const iot_command_msg_t *cmd)
{
    return iot_profile_dispatch_cmd(cmd);
}

iot_err_t iot_profile_robotics_init(void)
{
    iot_uart_cfg_t u1 = {
        .port = IOT_UART_PORT_1,
        .tx_pin = IOT_PIN_UART1_TX,
        .rx_pin = IOT_PIN_UART1_RX,
        .baud = 115200,
        .rs485_de_enable = false,
        .de_pin = -1,
    };
    (void)iot_uart_init(&u1);
    IOT_LOGI("robot", "UART telemetry from robot MCU; this node is a gateway");
    return IOT_OK;
}

iot_err_t iot_profile_robotics_sensor(iot_telemetry_msg_t *out)
{
    uint8_t buf[32];
    size_t n = 0;
    iot_uc_out_t uco;
    (void)iot_uart_read(IOT_UART_PORT_1, buf, sizeof(buf), &n, 10U);
    (void)iot_uc_robot_on_uart(n, &uco);
    return iot_services_fill_telemetry(out, uco.json);
}

iot_err_t iot_profile_robotics_cmd(const iot_command_msg_t *cmd)
{
    if ((cmd != NULL) && (strstr((const char *)cmd->payload, "move") != NULL)) {
        iot_uc_out_t o;
        return iot_uc_run_json("{\"uc\":\"robot.reject_motion\"}", 32, &o);
    }
    return iot_profile_dispatch_cmd(cmd);
}
