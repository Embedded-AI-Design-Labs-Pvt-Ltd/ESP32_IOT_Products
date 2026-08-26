idf_component_register(
    SRCS
        "src/iot_err.c"
        "src/iot_log.c"
        "src/iot_config.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash
    PRIV_REQUIRES log
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror=implicit-function-declaration)
