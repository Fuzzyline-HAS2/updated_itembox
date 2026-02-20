"""
QC Rules Test Suite
===================
QC 시스템이 하드웨어 오류 및 시스템 이상을 올바르게 감지하는지 테스트.
이 테스트가 통과하면, 실제 하드웨어 오류 발생 시 펌웨어가 이를 보고할 수 있음이 보장됨.
"""

from tests.itembox_model import State

def test_qc_memory_leak(model):
    """
    [SYS_MEM_01] 메모리 부족 감지 테스트
    - Heap이 10KB 미만이면 FAIL
    - Heap이 20KB 미만이면 WARN
    """
    # 1. Normal State
    model.free_heap = 50000
    results = model.run_qc_check()
    assert not any(r["id"] == "SYS_MEM_01" for r in results)

    # 2. Warning State (15KB)
    model.free_heap = 15000
    results = model.run_qc_check()
    warnings = [r for r in results if r["id"] == "SYS_MEM_01" and r["level"] == "WARN"]
    assert len(warnings) == 1
    assert warnings[0]["value"] == 15000

    # 3. Fail State (5KB)
    model.free_heap = 5000
    results = model.run_qc_check()
    fails = [r for r in results if r["id"] == "SYS_MEM_01" and r["level"] == "FAIL"]
    assert len(fails) == 1
    assert fails[0]["value"] == 5000

def test_qc_wifi_rssi(model):
    """
    [NET_WIFI_01] WiFi 신호 강도 감지 테스트
    - RSSI < -85dBm: FAIL
    - RSSI < -75dBm: WARN
    """
    # 1. Good Signal (-60)
    model.wifi_rssi = -60
    results = model.run_qc_check()
    assert not any(r["id"] == "NET_WIFI_01" for r in results)

    # 2. Weak Signal (-80) -> WARN
    model.wifi_rssi = -80
    results = model.run_qc_check()
    warnings = [r for r in results if r["id"] == "NET_WIFI_01" and r["level"] == "WARN"]
    assert len(warnings) == 1

    # 3. Bad Signal (-90) -> FAIL
    model.wifi_rssi = -90
    results = model.run_qc_check()
    fails = [r for r in results if r["id"] == "NET_WIFI_01" and r["level"] == "FAIL"]
    assert len(fails) == 1

def test_qc_encoder_range(model):
    """
    [HW_ENC_01] 엔코더 하드웨어 값 범위 이탈 테스트
    - 정상 범위: 0 ~ 380
    - 이탈 시: WARN (하드웨어 불량 의심)
    """
    # 1. Normal Value
    model.encoder_value = 200
    results = model.run_qc_check()
    assert not any(r["id"] == "HW_ENC_01" for r in results)

    # 2. Out of Range (High)
    model.encoder_value = 500
    results = model.run_qc_check()
    fails = [r for r in results if r["id"] == "HW_ENC_01"]
    assert len(fails) == 1
    assert fails[0]["value"] == 500

    # 3. Out of Range (Low/Negative)
    model.encoder_value = -10
    results = model.run_qc_check()
    fails = [r for r in results if r["id"] == "HW_ENC_01"]
    assert len(fails) == 1

def test_qc_rfid_init(model):
    """
    [HW_RFID_01] 듀얼 RFID 초기화 상태 테스트
    - 둘 중 하나라도 초기화 실패 시 FAIL
    """
    # 1. Both Init OK
    model.rfid_init_attempted = True
    model.rfid_init_complete = [True, True]
    results = model.run_qc_check()
    assert not any(r["id"] == "HW_RFID_01" for r in results)

    # 2. Outer Failed
    model.rfid_init_complete = [False, True]
    results = model.run_qc_check()
    fails = [r for r in results if r["id"] == "HW_RFID_01" and r["level"] == "FAIL"]
    assert len(fails) == 1

    # 3. Both Failed
    model.rfid_init_complete = [False, False]
    results = model.run_qc_check()
    assert len([r for r in results if r["id"] == "HW_RFID_01"]) == 1

    # 4. Init not attempted -> skip (no FAIL even if incomplete)
    model.rfid_init_attempted = False
    model.rfid_init_complete = [False, False]
    results = model.run_qc_check()
    assert not any(r["id"] == "HW_RFID_01" for r in results)

def test_qc_logic_fsm_consistency(model):
    """
    [LOGIC_FSM_01] 서버 상태와 로컬 FSM 불일치 감지
    - 서버가 'open'인데 로컬이 'BOOT'이면 불일치
    """
    # 1. Consistent State
    model.transition_to(State.OPEN)
    model.my["device_state"] = "open"
    results = model.run_qc_check()
    assert not any(r["id"] == "LOGIC_FSM_01" for r in results)

    # 2. Inconsistent Logic
    # 서버는 'used'라고 하는데, 기기는 아직 'BOOT' 상태
    model.state = State.BOOT
    model.my["device_state"] = "used"
    
    results = model.run_qc_check()
    fails = [r for r in results if r["id"] == "LOGIC_FSM_01" and r["level"] == "WARN"]
    assert len(fails) == 1
    assert "Server:used" in fails[0]["value"]


def test_qc_reset_reason(model):
    """
    [SYS_RST_01] 리셋 원인 감지 테스트
    - Brownout → FAIL
    - WDT/PANIC → WARN
    - 정상 부팅 → PASS
    """
    # 1. Normal boot
    model.last_reset_reason = "ESP_RST_POWERON"
    results = model.run_qc_check()
    assert not any(r["id"] == "SYS_RST_01" for r in results)

    # 2. Brownout -> FAIL
    model.last_reset_reason = "ESP_RST_BROWNOUT"
    results = model.run_qc_check()
    fails = [r for r in results if r["id"] == "SYS_RST_01" and r["level"] == "FAIL"]
    assert len(fails) == 1

    # 3. WDT -> WARN
    model.last_reset_reason = "ESP_RST_WDT"
    results = model.run_qc_check()
    warns = [r for r in results if r["id"] == "SYS_RST_01" and r["level"] == "WARN"]
    assert len(warns) == 1

    # 4. Panic -> WARN
    model.last_reset_reason = "ESP_RST_PANIC"
    results = model.run_qc_check()
    warns = [r for r in results if r["id"] == "SYS_RST_01" and r["level"] == "WARN"]
    assert len(warns) == 1


def test_qc_pin_conflict(model):
    """
    [HW_PIN_01] 핀 충돌(정적) 감지 테스트
    - RELAY_PIN == VIBRATION_RANGE_PIN → FAIL
    - PN532_SS1 == PN532_SS2 → FAIL
    """
    # 1. Default config has RELAY_PIN == VIBRATION_RANGE_PIN == 14 → FAIL
    results = model.run_qc_check()
    pin_fails = [r for r in results if r["id"] == "HW_PIN_01"]
    assert len(pin_fails) >= 1
    assert "RELAY_PIN" in pin_fails[0]["value"]

    # 2. Fix pin conflict → PASS
    model.pin_config["VIBRATION_RANGE_PIN"] = 15  # different pin
    results = model.run_qc_check()
    assert not any(r["id"] == "HW_PIN_01" for r in results)

    # 3. SS pin conflict → FAIL
    model.pin_config["PN532_SS2"] = model.pin_config["PN532_SS1"]  # same SS
    results = model.run_qc_check()
    ss_fails = [r for r in results if r["id"] == "HW_PIN_01"]
    assert len(ss_fails) == 1
    assert "PN532" in ss_fails[0]["what"]


def test_qc_gpio_capability(model):
    """
    [HW_GPIO_01] GPIO 특성 불일치 감지 테스트
    - buttonPin이 34~39(input-only, no pull-up) → FAIL
    """
    # 1. Default: buttonPin=34 → FAIL
    results = model.run_qc_check()
    gpio_fails = [r for r in results if r["id"] == "HW_GPIO_01"]
    assert len(gpio_fails) == 1
    assert "GPIO34" in gpio_fails[0]["value"]

    # 2. Change to non-input-only pin → PASS
    model.pin_config["buttonPin"] = 13
    results = model.run_qc_check()
    assert not any(r["id"] == "HW_GPIO_01" for r in results)


def test_qc_rfid_heartbeat(model):
    """
    [HW_RFID_02] RFID 런타임 하트비트 테스트
    - RFID 필요 상태에서 ACK 없음 → WARN
    - ACK 5초 초과 → FAIL
    - 정상 → PASS
    """
    model.rfid_init_attempted = True

    # 1. Non-RFID state → 규칙 스킵
    model.state = State.BOOT
    results = model.run_qc_check()
    assert not any(r["id"] == "HW_RFID_02" for r in results)

    # 2. RFID 필요 상태, ACK 없음 → WARN
    model.state = State.ACTIVATE
    model.last_rfid_ack_ms = 0
    results = model.run_qc_check()
    warns = [r for r in results if r["id"] == "HW_RFID_02" and r["level"] == "WARN"]
    assert len(warns) == 1

    # 3. ACK 후 6초 경과 → FAIL
    model.last_rfid_ack_ms = 1.0  # ACK at 1s
    model.sim_time = 7.0          # now 7s → 6000ms idle
    results = model.run_qc_check()
    fails = [r for r in results if r["id"] == "HW_RFID_02" and r["level"] == "FAIL"]
    assert len(fails) == 1

    # 4. ACK 후 2초 경과 → PASS
    model.last_rfid_ack_ms = 6.0  # ACK at 6s
    model.sim_time = 7.0          # now 7s → 1000ms idle
    results = model.run_qc_check()
    assert not any(r["id"] == "HW_RFID_02" for r in results)
