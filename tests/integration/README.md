# Integration tests

| ID | Flow | Pass |
| --- | --- | --- |
| IT-PATH-01 | ISR→sem→sensor→queue→proc→mqtt | broker receives JSON |
| IT-PATH-02 | MQTT command→queue→cmd→GPIO valve | water profile valve GPIO changes |
| IT-PROV-01 | BLE JSON→NVS→wifi start | WIFI_IP_READY |
