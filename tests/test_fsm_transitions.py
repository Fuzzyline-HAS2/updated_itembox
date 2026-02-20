"""
FSM Transition Test Suite
=========================
정상적인 게임 흐름(Happy Path)과 비정상적인 상태 전이 시도를 테스트.
"""

from tests.itembox_model import State

def test_fsm_happy_path(model, server):
    """
    [Happy Path] 전체 게임 시나리오
    Setting -> Ready -> Activate -> Puzzle -> Solve -> Open -> Used
    """
    # 1. Setting
    server.send_game_state("setting")
    assert model.state == State.SETTING
    assert model.encoder_value == 165
    assert any(h.func == "BoxOpen" for h in model.hw_log)

    # 2. Ready (Battery Distribution)
    server.send_game_state("ready")
    assert model.state == State.READY
    # 배터리팩 분배 확인 (Outbox에 battery_pack 메시지 존재해야 함)
    bp_msgs = model.get_sends("battery_pack")
    assert len(bp_msgs) > 0

    # 3. Activate
    server.send_game_state("activate")
    assert model.state == State.ACTIVATE
    assert model.current_mode == "RfidLoopOutter"
    
    # 4. Player Tag (Start Puzzle)
    server.inject_tag(role="player", tag_data="p1")
    assert model.state == State.PUZZLE
    assert model.current_mode == "Puzzle"

    # 5. Solve Puzzle (3 correct answers)
    # 정답: 13, 43, 21 (기본값)
    answers = [13, 43, 21]
    for ans in answers:
        # 엔코더를 정답 위치로 이동 (값 = 정답 * 4)
        model.encoder_value = ans * 4
        res = model.submit_encoder(button_pressed=True)
        if ans == answers[-1]:
            assert res == "quiz_complete"
        else:
            assert res == "correct"

    assert model.state == State.QUIZ_COMPLETE
    assert model.rfid_mode == "PuzzleSolved"

    # 6. Confirm Tag (Open Box)
    # 퀴즈 완료 후 다시 태그 -> PuzzleSolved -> Open
    server.inject_tag(role="player")
    assert model.state == State.OPEN
    assert model.self_open is True
    assert model.last_neo_color() == 4  # BLUE

    # 7. Item Took (Used)
    # 다시 태그 -> ItemTook -> Used
    server.inject_tag(role="player", battery_pack=0, max_battery_pack=10)
    assert model.state == State.USED
    assert model.used is True
    assert model.last_neo_color() == 1  # RED

def test_fsm_invalid_transitions(model, server):
    """
    [Edge Case] 잘못된 상태 전이 시도
    """
    # 1. BOOT에서 바로 Player Win 같은 상태로 점프 가능? (Device State는 가능)
    server.send_device_state("player_win")
    assert model.state == State.PLAYER_WIN

    # 2. USED 상태에서 중복 USED 요청 -> 무시되어야 함
    # 먼저 정상적으로 USED 상태 진입 (used=True 설정됨)
    server.send_device_state("used")
    assert model.state == State.USED
    assert model.used is True

    # 이제 다시 USED 보내면 무시되어야 함
    model.hw_log.clear()
    server.send_device_state("used")
    # 로그가 비어있어야 함 (재진입 안 함)
    assert not any(h.func == "BoxOpen" for h in model.hw_log)

def test_fsm_reset_during_puzzle(model, server):
    """
    [Recovery] 퍼즐 도중 MMMM 태그(리셋) 
    """
    server.send_game_state("activate")
    server.inject_tag(role="player")
    assert model.state == State.PUZZLE
    
    # 리셋 태그
    server.inject_tag(role="unknown", tag_data="MMMM")
    assert model.state == State.BOOT
    assert model.encoder_value == 165  # 초기화 확인
