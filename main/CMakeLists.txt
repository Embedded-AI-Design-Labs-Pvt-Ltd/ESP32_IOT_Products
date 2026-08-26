idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        iot_core iot_hal iot_drivers iot_rtos iot_wifi iot_ble iot_mqtt
        iot_security iot_ota iot_diag iot_provision iot_services iot_profiles iot_usecases
        nvs_flash
)
