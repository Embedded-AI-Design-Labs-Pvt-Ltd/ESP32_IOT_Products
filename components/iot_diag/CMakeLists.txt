idf_component_register(
    SRCS "src/iot_diag.c"
    INCLUDE_DIRS "include"
    REQUIRES iot_core iot_rtos iot_hal
)
