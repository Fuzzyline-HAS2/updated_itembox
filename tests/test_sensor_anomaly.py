"""
Sensor Anomaly Test Suite
=========================
센서 값 이상(튀는 값, 끊김, 노이즈)에 대한 QC 시스템 반응 테스트.
"""

from tests.itembox_model import State
from tests.sensor_gen import generate_spike, generate_noise

def test_encoder_spike(model):
    """
    [Anomaly] 엔코더 값이 순간적으로 튀었을 때 (Glitch)
    """
    # 0. RFID 초기화 완료로 설정 (엔코더만 테스트하기 위해)
    model.rfid_init_complete = [True, True]

    # 1. Normal
    model.encoder_value = 100
    assert not any(r["level"] != "PASS" for r in model.run_qc_check())
    
    # 2. Spike (Glitch) - 999
    # Python 모델은 스트림을 직접 받지 않고 loop()에서 값을 읽는다고 가정.
    # 여기서는 값을 강제로 할당하여 시뮬레이션.
    model.encoder_value = 999
    
    # QC Check -> HW_ENC_01 WARN
    results = model.run_qc_check()
    warnings = [r for r in results if r["id"] == "HW_ENC_01"]
    assert len(warnings) == 1
    
    # 3. Recovery
    model.encoder_value = 100
    assert not any(r["level"] != "PASS" for r in model.run_qc_check())

def test_rfid_timeout(model, server):
    """
    [Anomaly] RFID 태그가 너무 오래 없는 경우 (Keep Alive)
    (이 기능은 펌웨어에 명시적으로 없지만, QC 관점에서 테스트)
    """
    pass # 현재 구현된 로직 없음. Placeholder.

def test_ghost_tag_behavior(model, server):
    """
    [Anomaly] 'ghost' 태그 (미등록/알수없는 태그) 시 동작
    """
    server.send_game_state("activate")
    
    # Ghost Tag
    model.serial_log.clear()
    server.inject_tag(role="ghost")
    
    # 로그 확인
    assert "Ghost Tagged" in model.serial_log
    # 상태 변화 없음
    assert model.state == State.ACTIVATE

def test_unknown_tag_behavior(model, server):
    """
    [Anomaly] role이 없는 완전 이상한 태그
    """
    server.inject_tag(role="alien")
    assert "Wrong TAG" in model.serial_log
