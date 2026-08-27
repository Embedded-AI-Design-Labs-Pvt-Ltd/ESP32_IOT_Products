/**
 * @file iot_port.h
 * @copyright Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
 * @author    Muhammad Samiullah, CTO & Founder
 * @brief Target vs host compilation switch. Application code includes this, not ESP-IDF.
 */
#ifndef IOT_PORT_H
#define IOT_PORT_H

#if defined(ESP_PLATFORM)
#ifndef IOT_TARGET_ESP32C3
#define IOT_TARGET_ESP32C3 1
#endif
#else
#ifndef IOT_HOST_SIM
#define IOT_HOST_SIM 1
#endif
#endif

#endif /* IOT_PORT_H */
