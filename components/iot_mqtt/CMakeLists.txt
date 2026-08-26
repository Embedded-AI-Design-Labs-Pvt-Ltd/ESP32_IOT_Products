idf_component_register(
    SRCS "src/iot_mqtt.c"
    INCLUDE_DIRS "include"
    REQUIRES iot_core iot_rtos mqtt esp_http_client
    PRIV_REQUIRES esp-tls mbedtls esp_crt_bundle
)
