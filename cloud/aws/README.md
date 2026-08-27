# AWS IoT Core overlay

Use MQTTS to the device data endpoint. Client id = device id.

Do not store certs in git. Place PEM files on the device via manufacturing NVS or a provisioning partition.

```
mqtt_uri = mqtts://<endpoint>.iot.<region>.amazonaws.com:8883
```

Topics already match `device/{id}/#`. Map AWS IoT policies to those topics. Use JITP or fleet provisioning in the factory — not hardcoded keys.

Example policy resource: `arn:aws:iot:region:acct:topic/device/${iot:ClientId}/*`

---

**Embedded AI Labs** — *Intelligence at the Edge*  
ಎಂಬೆಡೆಡ್ ಎಐ ಲ್ಯಾಬ್ಸ್  
**Embedded AI Design Labs Pvt Ltd**  
Muhammad Samiullah — CTO & Founder  
© 2026 Copyright. All rights reserved.
