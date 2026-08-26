idf_component_register(
    SRCS "src/iot_services.c"
    INCLUDE_DIRS "include"
    REQUIRES iot_core iot_security iot_provision iot_ota iot_diag iot_usecases
)
