# Use-case catalog

Every product and platform flow is a named C handler (`iot_uc_run` / `iot_uc_run_json`) plus a C++ `iot::UseCaseEngine`.

Command JSON:

```json
{"uc":"energy.set_limits","over_w":3500,"over_v":253,"under_v":207,"over_i":16}
{"uc":"water.valve_close"}
{"uc":"ev.contactor_close"}  
```

`ev.reject_contactor`, `auto.reject_tx`, and `robot.reject_motion` always return `IOT_ERR_DENIED`.

See `docs/usecases/README.md` for the full list.
