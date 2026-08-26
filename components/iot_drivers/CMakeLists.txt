idf_component_register(
    SRCS
        "src/iot_gpio.c"
        "src/iot_uart.c"
        "src/iot_i2c.c"
        "src/iot_spi.c"
        "src/iot_adc.c"
        "src/iot_pwm.c"
        "src/iot_timer.c"
        "src/iot_exti.c"
        "src/iot_wdt.c"
        "src/iot_drivers.c"
    INCLUDE_DIRS "include"
    REQUIRES iot_core iot_rtos iot_hal driver esp_adc
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra)
