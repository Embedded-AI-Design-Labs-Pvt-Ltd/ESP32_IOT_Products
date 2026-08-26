/**
 * @file iot_version.h
 * @brief Firmware semantic version consumed by OTA and BLE Device Information.
 */
#ifndef IOT_VERSION_H
#define IOT_VERSION_H

#ifndef IOT_FW_VERSION_MAJOR
#define IOT_FW_VERSION_MAJOR 1
#endif
#ifndef IOT_FW_VERSION_MINOR
#define IOT_FW_VERSION_MINOR 0
#endif
#ifndef IOT_FW_VERSION_PATCH
#define IOT_FW_VERSION_PATCH 0
#endif

#define IOT_FW_VERSION_STRING "1.0.0"
#define IOT_PLATFORM_NAME "esp32c3-iot-platform"

#endif /* IOT_VERSION_H */
