"""
Idempotency Test Suite
======================
네트워크 메시지 중복, 재전송, 타임아웃 처리에 대한 멱등성 검증.
동일한 명령을 여러 번 받아도 시스템 상태가 1번만 변경되어야 함.
"""

from tests.itembox_model import State

def test_setting_idempotency(model, server):
    """
    [Idempotency] 'setting' 명령 중복 수신
    """
    # 1. First Call
    server.send_game_state("setting")
    assert model.state == State.SETTING
    # BoxOpen 호출 1회 확인
    assert len(model.get_hw_calls("BoxOpen")) == 1
    
    # 2. Duplicate Call
    server.send_game_state("setting")
    assert model.state == State.SETTING
    # BoxOpen 호출 횟수가 여전히 1회여야 함 (상태 변경 없으므로)
    assert len(model.get_hw_calls("BoxOpen")) == 1

def test_open_idempotency(model, server):
    """
    [Idempotency] 'open' 명령 중복 수신
    """
    # 초기 상태: Puzzle Complete
    server.send_game_state("activate")
    # Puzzle setup needs player tag
    server.inject_tag(role="player")
    
    # Force solve quiz to allow natural transition if needed, 
    # but strictly speaking device_state="open" forces it anyway.
    
    # 1. First Open
    model.hw_log = [] # Clear log to count fresh calls
    server.send_device_state("open")
    assert model.state == State.OPEN
    
    # BoxOpen should be called once
    open_calls_1 = len(model.get_hw_calls("BoxOpen"))
    assert open_calls_1 == 1
    
    # 2. Duplicate Open
    server.send_device_state("open")
    assert model.state == State.OPEN
    
    # Count should not increase
    open_calls_2 = len(model.get_hw_calls("BoxOpen"))
    assert open_calls_2 == open_calls_1

def test_used_idempotency(model, server):
    """
    [Idempotency] 'used' 명령 중복 수신
    """
    # Setup: Open state via server
    server.send_device_state("open")
    assert model.state == State.OPEN
    model.self_open = True # Logic sets this, but let's be sure state is clean
    
    # Clear outbox to count new messages
    model.outbox = [] 
    
    # 1. First Used
    server.send_device_state("used")
    assert model.state == State.USED
    assert model.used is True
    
    # Check if 'used' confirmation sent
    # Wifi.ino logic: when receiving "used" from server, it does NOT send "device_state"="used" back.
    # It only sends it when RFID "ItemTook" happens.
    sends_1 = model.get_sends("device_state")
    used_msgs_1 = [m for m in sends_1 if m.value == "used"]
    assert len(used_msgs_1) == 0
    
    # 2. Duplicate Used
    server.send_device_state("used")
    assert model.state == State.USED
    
    # Sends count should still be 0
    sends_2 = model.get_sends("device_state")
    used_msgs_2 = [m for m in sends_2 if m.value == "used"]
    assert len(used_msgs_2) == 0
