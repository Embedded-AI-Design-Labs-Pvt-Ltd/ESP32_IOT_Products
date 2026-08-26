# SPDX-License-Identifier: Apache-2.0
# ESP32-C3 commercial IoT platform — ESP-IDF project root.

cmake_minimum_required(VERSION 3.16)

set(COMPONENTS main)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp32c3_iot_platform)

# Record IDF version into a header consumed by diagnostics and OTA.
idf_build_get_property(idf_ver IDF_VER)
add_compile_definitions(IOT_IDF_VERSION="${idf_ver}")
