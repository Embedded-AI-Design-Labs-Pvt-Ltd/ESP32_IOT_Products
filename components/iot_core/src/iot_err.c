/**
 * @file iot_err.c
 * @brief Error-code translation and naming.
 *
 * @syscalls none
 * @thread any (reentrant)
 */
#include "iot_err.h"

#ifdef ESP_PLATFORM
#include "esp_err.h"
#endif

iot_err_t iot_err_from_esp(int esp_code)
{
    if (esp_code == 0) {
        return IOT_OK;
    }
#ifdef ESP_PLATFORM
    switch (esp_code) {
    case ESP_ERR_NO_MEM:
        return IOT_ERR_NO_MEM;
    case ESP_ERR_INVALID_ARG:
        return IOT_ERR_INVALID_ARG;
    case ESP_ERR_INVALID_STATE:
        return IOT_ERR_INVALID_STATE;
    case ESP_ERR_NOT_FOUND:
        return IOT_ERR_NOT_FOUND;
    case ESP_ERR_NOT_SUPPORTED:
        return IOT_ERR_NOT_SUPPORTED;
    case ESP_ERR_TIMEOUT:
        return IOT_ERR_TIMEOUT;
    case ESP_ERR_INVALID_CRC:
        return IOT_ERR_CRC;
    default:
        return IOT_ERR_FAIL;
    }
#else
    (void)esp_code;
    return IOT_ERR_FAIL;
#endif
}

const char *iot_err_to_name(iot_err_t err)
{
    switch (err) {
    case IOT_OK:
        return "IOT_OK";
    case IOT_ERR_FAIL:
        return "IOT_ERR_FAIL";
    case IOT_ERR_INVALID_ARG:
        return "IOT_ERR_INVALID_ARG";
    case IOT_ERR_NO_MEM:
        return "IOT_ERR_NO_MEM";
    case IOT_ERR_TIMEOUT:
        return "IOT_ERR_TIMEOUT";
    case IOT_ERR_BUSY:
        return "IOT_ERR_BUSY";
    case IOT_ERR_NOT_FOUND:
        return "IOT_ERR_NOT_FOUND";
    case IOT_ERR_NOT_SUPPORTED:
        return "IOT_ERR_NOT_SUPPORTED";
    case IOT_ERR_INVALID_STATE:
        return "IOT_ERR_INVALID_STATE";
    case IOT_ERR_IO:
        return "IOT_ERR_IO";
    case IOT_ERR_CRC:
        return "IOT_ERR_CRC";
    case IOT_ERR_AUTH:
        return "IOT_ERR_AUTH";
    case IOT_ERR_DENIED:
        return "IOT_ERR_DENIED";
    case IOT_ERR_OVERFLOW:
        return "IOT_ERR_OVERFLOW";
    case IOT_ERR_AGAIN:
        return "IOT_ERR_AGAIN";
    case IOT_ERR_HW:
        return "IOT_ERR_HW";
    case IOT_ERR_NOT_INIT:
        return "IOT_ERR_NOT_INIT";
    case IOT_ERR_ALREADY_INIT:
        return "IOT_ERR_ALREADY_INIT";
    case IOT_ERR_QUEUE_FULL:
        return "IOT_ERR_QUEUE_FULL";
    case IOT_ERR_QUEUE_EMPTY:
        return "IOT_ERR_QUEUE_EMPTY";
    case IOT_ERR_PROTOCOL:
        return "IOT_ERR_PROTOCOL";
    case IOT_ERR_TLS:
        return "IOT_ERR_TLS";
    case IOT_ERR_OTA:
        return "IOT_ERR_OTA";
    case IOT_ERR_WATCHDOG:
        return "IOT_ERR_WATCHDOG";
    case IOT_ERR_ISR:
        return "IOT_ERR_ISR";
    default:
        return "IOT_ERR_UNKNOWN";
    }
}
