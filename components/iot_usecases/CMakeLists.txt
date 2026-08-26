idf_component_register(
    SRCS
        "src/iot_uc.c"
        "src/iot_uc_platform.c"
        "src/iot_uc_energy.c"
        "src/iot_uc_industrial.c"
        "src/iot_uc_water.c"
        "src/iot_uc_ev_auto.c"
        "src/iot_uc_field.c"
        "src/iot_uc_catalog.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "src"
    REQUIRES iot_core iot_rtos iot_drivers iot_hal iot_diag iot_provision iot_ota iot_security iot_mqtt
)
