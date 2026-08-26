# GCP-compatible MQTT

Google Cloud IoT Core is **deprecated**. Use a self-managed MQTTS broker on GCP (Cloud VM, MQTT on GKE, or a partner broker) or another cloud.

Set `mqtt_uri` to that broker. Identity is still the NVS device id + TLS client cert if the broker requires mTLS.

Do not use expired IoT Core device registries.
