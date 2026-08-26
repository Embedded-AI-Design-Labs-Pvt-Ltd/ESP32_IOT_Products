idf_component_register(
    SRCS "src/iot_ble.c"
    INCLUDE_DIRS "include"
    REQUIRES iot_core iot_rtos bt nvs_flash
)
