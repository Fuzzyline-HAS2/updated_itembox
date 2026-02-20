"""
Reset & Recovery Test Suite
===========================
시스템 리셋, 부팅 복구, 중간 리셋 시나리오 테스트.
"""

from tests.itembox_model import State

def test_reset_during_game(model, server):
    """
    [Recovery] 게임 도중 리셋 시 초기 상태 복구 확인
    """
    server.send_game_state("activate")
    assert model.state == State.ACTIVATE
    
    # Reset
    model.reset()
    
    assert model.state == State.BOOT
    assert model.wifi_connected is True
    assert model.encoder_value == 165

def test_reconnect_recovery(model, server):
    """
    [Recovery] WiFi 재연결 후 상태 동기화 확인
    """
    # 1. Disconnect
    model.wifi_connected = False
    results = model.run_qc_check()
    assert any(r["id"] == "NET_WIFI_00" for r in results)
    
    # 2. Reconnect
    model.wifi_connected = True
    results = model.run_qc_check()
    assert not any(r["id"] == "NET_WIFI_00" for r in results)
