"""
Network Fault Test Suite
========================
네트워크 지연, 드랍, 중복 발생 시 시스템 안정성 테스트.
"""

import time
import pytest
from tests.itembox_model import State

def test_network_delay(model, shim_server):
    """
    [Network] 200ms 지연 시에도 상태 전이 정상 동작 확인
    """
    server, shim_msg, _ = shim_server
    
    # 200ms 지연 설정
    shim_msg.delay_ms = 200
    
    start_time = time.time()
    server.send_game_state("setting")
    elapsed = time.time() - start_time
    
    assert elapsed >= 0.2
    assert model.state == State.SETTING

def test_network_drop_retry_sim(model, shim_server):
    """
    [Network] 패킷 드랍 시 상태 불일치 발생 확인
    (현재 펌웨어엔 재시도 로직이 없으므로, 드랍되면 상태가 안 바뀜을 검증)
    """
    server, shim_msg, _ = shim_server
    
    # 100% 드랍 설정
    shim_msg.drop_rate = 1.0
    
    # Activate 요청 보냈으나 드랍됨
    server.send_game_state("activate")
    
    # 상태가 여전히 BOOT여야 함 (변경 안 됨)
    assert model.state == State.BOOT
    
    # 드랍 해제 후 재전송
    shim_msg.drop_rate = 0.0
    server.send_game_state("activate")
    assert model.state == State.ACTIVATE

def test_network_duplicate(model, shim_server):
    """
    [Idempotency] 메시지 중복 수신 시 멱등성 확인
    """
    server, shim_msg, _ = shim_server
    
    # 중복 확률 100% (항상 2번 전송)
    # NetShim 구현상 original + duplicate = 2번 호출
    shim_msg.duplicate_prob = 1.0
    
    # Setting 명령 전송
    model.hw_log.clear()
    server.send_game_state("setting")
    
    assert model.state == State.SETTING
    # BoxOpen은 1번만 호출되는 것이 이상적 (하지만 State check가 있으면 중복 호출 막음)
    # transition_to 함수가 (current == new)면 return false 하므로
    # 실제로는 1번만 로그 발생해야 함.
    box_opens = [h for h in model.hw_log if h.func == "BoxOpen"]
    assert len(box_opens) == 1
