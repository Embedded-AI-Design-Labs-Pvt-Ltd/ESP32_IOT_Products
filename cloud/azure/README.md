# Azure IoT Hub overlay

IoT Hub MQTT: `mqtts://<hub>.azure-devices.net:8883`

Username: `{hub}.azure-devices.net/{deviceId}/?api-version=2021-04-12`

Password: SAS token generated at provisioning time — never compile it in.

Map:

- telemetry → `devices/{id}/messages/events/` (adjust `iot_mqtt_build_topic` in a thin overlay if Hub routing is required)
- cloud-to-device → subscribe Hub C2D topic

This platform's default topic layout is generic MQTT. For Hub, set `IOT_CLOUD_AZURE_IOT` and extend `iot_mqtt.c` with Hub username format in a follow-up — keep secrets in NVS.
