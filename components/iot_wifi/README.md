# iot_wifi

States: DISCONNECTED, CONNECTING, CONNECTED, IP_READY, ERROR, RECONNECTING.

Thread-safe `iot_wifi_get_status`. MQTT must wait for `IOT_EVT_WIFI_IP`.
