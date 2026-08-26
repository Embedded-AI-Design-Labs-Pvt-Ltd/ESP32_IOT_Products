# Fault injection

| ID | Inject | Expected |
| --- | --- | --- |
| FT-MQTT-01 | kill broker | DIAG_MQTT_DISCONNECT, reconnect |
| FT-OTA-01 | http URL | IOT_ERR_DENIED |
| FT-CMD-01 | empty payload | IOT_ERR_DENIED |
| FT-WDT-01 | hang sensor task | TASK_WDT reset |
