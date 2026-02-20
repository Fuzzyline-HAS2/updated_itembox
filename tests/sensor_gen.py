"""
Sensor Stream Generator
=======================
센서(Encoder, RFID)의 정상/비정상 데이터 스트림을 생성하여
모델에 주입하는 유틸리티.
"""

from typing import Iterator

def generate_encoder_answers(answers: list[int],
                             range_val: int = 2) -> Iterator[int]:
    """
    정답 순서대로 엔코더 값을 생성하는 제너레이터.
    각 정답에 대해 (정답 * 4) 근처의 값을 생성.
    """
    for ans in answers:
        # 정답 범위 내의 랜덤 값 (여기서는 정확히 중앙값 사용)
        target_val = ans * 4
        # 주변 값을 흔들면서 정답에 도달하는 시나리오라면 더 복잡하겠지만
        # 여기서는 단순 정답 입력 시퀀스
        yield target_val

def generate_noise(base_val: int, intensity: int = 5, count: int = 10) -> Iterator[int]:
    """
    기준값 주변에서 노이즈가 섞인 값을 생성.
    """
    import random
    for _ in range(count):
        yield base_val + random.randint(-intensity, intensity)

def generate_spike(base_val: int, spike_val: int) -> Iterator[int]:
    """
    갑자기 튀는 값 생성 (스파이크).
    """
    yield base_val
    yield spike_val
    yield base_val

def generate_drift(start: int, end: int, step: int = 1) -> Iterator[int]:
    """
    값이 서서히 변하는 드리프트 현상 생성.
    """
    current = start
    while current != end:
        yield current
        if start < end:
            current += step
            if current > end: current = end
        else:
            current -= step
            if current < end: current = end
    yield end
