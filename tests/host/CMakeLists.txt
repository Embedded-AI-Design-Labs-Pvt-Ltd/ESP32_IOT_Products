cmake_minimum_required(VERSION 3.16)
project(iot_host_tests C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(IOT_HOST_SIM=1)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_executable(iot_host_tests
    test_runner.c
    ${ROOT}/components/iot_core/src/iot_err.c
    ${ROOT}/components/iot_core/src/iot_log.c
    ${ROOT}/components/iot_core/src/iot_config.c
    ${ROOT}/components/iot_rtos/src/iot_rtos.c
    ${ROOT}/components/iot_rtos/src/iot_tasks.c
    ${ROOT}/components/iot_drivers/src/iot_gpio.c
    ${ROOT}/components/iot_drivers/src/iot_adc.c
    ${ROOT}/components/iot_drivers/src/iot_wdt.c
    ${ROOT}/components/iot_drivers/src/iot_pwm.c
    ${ROOT}/components/iot_security/src/iot_security.c
    ${ROOT}/components/iot_diag/src/iot_diag.c
    ${ROOT}/components/iot_provision/src/iot_provision.c
    ${ROOT}/components/iot_ota/src/iot_ota.c
    ${ROOT}/components/iot_services/src/iot_services.c
    ${ROOT}/components/iot_mqtt/src/iot_mqtt.c
    ${ROOT}/components/iot_hal/src/iot_board.c
    ${ROOT}/components/iot_profiles/src/iot_profile_energy.c
    ${ROOT}/components/iot_usecases/src/iot_uc.c
    ${ROOT}/components/iot_usecases/src/iot_uc_platform.c
    ${ROOT}/components/iot_usecases/src/iot_uc_energy.c
    ${ROOT}/components/iot_usecases/src/iot_uc_industrial.c
    ${ROOT}/components/iot_usecases/src/iot_uc_water.c
    ${ROOT}/components/iot_usecases/src/iot_uc_ev_auto.c
    ${ROOT}/components/iot_usecases/src/iot_uc_field.c
    ${ROOT}/components/iot_usecases/src/iot_uc_catalog.cpp
)

target_include_directories(iot_host_tests PRIVATE
    ${ROOT}/components/iot_core/include
    ${ROOT}/components/iot_rtos/include
    ${ROOT}/components/iot_drivers/include
    ${ROOT}/components/iot_hal/include
    ${ROOT}/components/iot_security/include
    ${ROOT}/components/iot_diag/include
    ${ROOT}/components/iot_provision/include
    ${ROOT}/components/iot_services/include
    ${ROOT}/components/iot_mqtt/include
    ${ROOT}/components/iot_ota/include
    ${ROOT}/components/iot_profiles/include
    ${ROOT}/components/iot_usecases/include
    ${ROOT}/components/iot_usecases/src
)

if (WIN32)
    target_link_libraries(iot_host_tests PRIVATE)
else()
    target_link_libraries(iot_host_tests PRIVATE pthread m)
endif()

enable_testing()
add_test(NAME host_unit COMMAND iot_host_tests)
