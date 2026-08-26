idf_component_register(
    SRCS "src/iot_wifi.c"
    INCLUDE_DIRS "include"
    REQUIRES iot_core iot_rtos esp_wifi esp_netif esp_event
)
