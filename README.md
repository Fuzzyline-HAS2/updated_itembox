# ItemBox QC and Python Test Harness

This README is focused on the QC runtime code in `QC/` and the Python test harness in `tests/`.

## Scope
- Runtime quality checks on device (ESP32 firmware side)
- Hardware-free regression testing on PC with pytest
- Rule parity between C++ QC rules and Python model QC checks

## QC Runtime (C++)

### Files
- `QC/QC_Types.h`
  - `QCLevel`: `PASS`, `WARN`, `FAIL`
  - `QCResult`: `ruleId`, `what`, `criterion`, `value`, `fix`, `timestamp`
  - `IQCRule` interface
- `QC/QC_Engine.h`
  - rule registration and execution engine
  - separates `FAST` (every tick) and `SLOW` (interval) rules
- `QC/QC_Rules.h`
  - concrete QC rule implementations

### Firmware integration points
- `updated_itembox.ino`
  - initialize QC engine in `setup()`
  - register rules with `addRule(...)`
  - call `QCEngine::getInstance().tick()` in `loop()`

### QC log format
- `[FAIL] [RULE_ID] what value (Limit: criterion) -> Fix: ...`
- `[WARN] [RULE_ID] what value (Limit: criterion) -> Fix: ...`

### Implemented rule IDs (current)
1. `NET_WIFI_00` WiFi connection status (FAST)
2. `NET_WIFI_01` WiFi RSSI threshold (SLOW)
3. `SYS_MEM_01` free heap threshold (SLOW)
4. `SYS_RST_01` reset reason check (Brownout/WDT/Panic) (SLOW)
5. `LOGIC_STATE_01` server state value validity (SLOW)
6. `HW_RFID_01` dual RFID init status (SLOW)
7. `HW_RFID_02` RFID heartbeat (ACK timeout) (SLOW)
8. `HW_PIN_01` static pin conflict check (FAST)
9. `HW_GPIO_01` GPIO capability mismatch check (FAST)
10. `HW_ENC_01` encoder range check (SLOW)
11. `LOGIC_FSM_01` FSM vs server state consistency (SLOW)

## Python Test Harness (pytest)

### Files
- `tests/itembox_model.py`
  - pure-Python FSM model of firmware behavior
  - logs hardware-side effects in `hw_log`
  - logs outgoing network messages in `outbox`
  - includes Python version of the 11 QC rules via `run_qc_check()`
- `tests/fake_server.py`
  - injects server messages and RFID events
- `tests/net_shim.py`
  - fault injection: delay, drop, duplicate
- `tests/sensor_gen.py`
  - synthetic sensor stream helpers
- `tests/conftest.py`
  - fixtures: `model`, `server`, `shim_server`

### Test suites
- `tests/test_fsm_transitions.py`
- `tests/test_idempotency.py`
- `tests/test_network_faults.py`
- `tests/test_qc_rules.py`
- `tests/test_reset_recovery.py`
- `tests/test_sensor_anomaly.py`

## How to run tests

```powershell
python -m pytest tests/ -v -p no:cacheprovider
```

## Current local result
- 24 collected
- 23 passed
- 1 failed: `tests/test_sensor_anomaly.py::test_encoder_spike`

Failure note:
- The test expects "no non-PASS QC result" in a nominal encoder case.
- Default model pin setup currently triggers other QC failures (`HW_PIN_01`, `HW_GPIO_01`), so the assertion is too broad.
- The test should filter specifically for `HW_ENC_01` if encoder-only behavior is intended.

## Operational notes
- `timer.ino` should call `has2wifi.Loop(DataChanged)` inside `WifiIntervalFunc()`.
- Save source files as `UTF-8 without BOM` to avoid Arduino compile errors from hidden BOM bytes.
