"""
Fake Server for ItemBox
=======================
HAS2 WiFi 서버의 동작을 모방하여 JSON 이벤트를 생성하고
ItemBox 모델의 상태를 변경하는 이벤트를 주입.
"""

from __future__ import annotations
from typing import Callable, Any

# Define callback type for type hinting
ModelCallback = Callable[..., None]
RfidCallback = Callable[..., None]

class FakeServer:
    """
    가짜 HAS2 서버.
    테스트 코드에서 이 서버를 통해 ItemBox 모델에 명령을 내림.
    """
    
    def __init__(self):
        self.model_msg_callback: ModelCallback | None = None
        self.model_rfid_callback: RfidCallback | None = None

    def bind(self, msg_callback: ModelCallback, rfid_callback: RfidCallback):
        """ItemBox 모델의 수신부(on_server_message, rfid_tag)와 연결"""
        self.model_msg_callback = msg_callback
        self.model_rfid_callback = rfid_callback

    def _send_msg(self, **data):
        """기기로 JSON 메시지 전송 (모델의 on_server_message 호출)"""
        if self.model_msg_callback:
            self.model_msg_callback(**data)

    def _send_rfid(self, **data):
        """기기로 RFID 태그 이벤트 전송 (모델의 rfid_tag 호출)"""
        if self.model_rfid_callback:
            self.model_rfid_callback(**data)

    # ── Server Commands (WiFi) ──────────────────────────────────
    def send_game_state(self, state: str):
        """관리자: game_state 변경 (setting, ready, activate)"""
        self._send_msg(game_state=state)

    def send_device_state(self, state: str, device_name: str = "itembox"):
        """서버: device_state 변경 (force close, repaired_all 등)"""
        self._send_msg(device_state=state, device_name=device_name)

    def send_manage_state(self, state: str):
        """관리자: manage_state 변경 (mo, mu)"""
        self._send_msg(manage_state=state)

    def send_battery_pack(self, count: int):
        """서버: 보유 배터리팩 수량 업데이트"""
        self._send_msg(battery_pack=str(count))

    # ── Sensor Events (Inputs) ──────────────────────────────────
    def inject_tag(self, role: str = "player", 
                   tag_data: str = "G1P1",
                   battery_pack: int = 0,
                   max_battery_pack: int = 10,
                   device_name: str = "player1",
                   exp_pack: str = "0"):
        """
        RFID 태그 이벤트 주입.
        CheckingPlayers()에서 RFID 태그 시 발생하는 동작 시뮬레이션.
        """
        self._send_rfid(
            role=role,
            tag_data=tag_data,
            battery_pack=battery_pack,
            max_battery_pack=max_battery_pack,
            device_name=device_name,
            exp_pack=exp_pack
        )
