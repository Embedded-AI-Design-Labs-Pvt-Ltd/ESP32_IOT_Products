idf_component_register(
    SRCS "src/iot_ota.c"
    INCLUDE_DIRS "include"
    REQUIRES iot_core iot_rtos iot_drivers app_update esp_https_ota esp_http_client
)
