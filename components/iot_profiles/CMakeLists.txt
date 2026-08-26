idf_component_register(
    SRCS
        "src/iot_profile.c"
        "src/iot_profile_energy.c"
        "src/iot_profile_industrial.c"
        "src/iot_profile_predictive.c"
        "src/iot_profile_water.c"
        "src/iot_profile_ev.c"
        "src/iot_profile_automotive.c"
        "src/iot_profile_rest.c"
    INCLUDE_DIRS "include"
    REQUIRES iot_core iot_rtos iot_drivers iot_hal iot_services iot_usecases
)
