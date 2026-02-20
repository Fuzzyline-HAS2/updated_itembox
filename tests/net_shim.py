"""
Network Fault Shim
==================
메시지 전송 과정에 네트워크 장애(지연, 드랍, 중복)를
시뮬레이션하는 중간 계층(Proxy).
"""

import time
import random
from dataclasses import dataclass
from typing import Callable, Any

@dataclass
class NetShim:
    """
    네트워크 장애 주입기.
    send() 호출 시 설정된 확률로 드랍하거나 지연시킴.
    """
    target: Callable[..., Any]
    drop_rate: float = 0.0          # 0.0 ~ 1.0 (1.0 = 100% drop)
    delay_ms: int = 0               # 지연 시간 (ms)
    duplicate_prob: float = 0.0     # 중복 전송 확률

    def send(self, **kwargs):
        """메시지 전송 (장애 시뮬레이션 포함)"""
        
        # 1. Packet Drop
        if random.random() < self.drop_rate:
            print(f"[NetShim] Packet DROPPED: {kwargs}")
            return  # 전송 안 함

        # 2. Network Delay
        if self.delay_ms > 0:
            time.sleep(self.delay_ms / 1000.0)

        # 3. Normal Send
        self.target(**kwargs)

        # 4. Duplicate Packet
        if random.random() < self.duplicate_prob:
            print(f"[NetShim] Packet DUPLICATED: {kwargs}")
            self.target(**kwargs)
