# 임베디드 프로젝트용 QC + Python 테스트 하네스 구축 프롬프트

> 이 문서는 AI 코딩 어시스턴트에게 붙여넣기 하면,
> 현재 레포의 펌웨어를 분석하여 **하드웨어 QC 시스템**(C++)과 **Python 테스트 하네스**(pytest)를 동시에 만들어주는 프롬프트입니다.

---

## 사용법

1. AI 어시스턴트(Gemini, Claude 등)에게 아래 프롬프트를 전달
2. 레포 루트 경로를 알려줌
3. 생성된 코드 리뷰 후 적용

---

## 프롬프트 (여기부터 복사)

```
아래 두 시스템을 이 레포에 만들어줘.
절대 기존 .ino 로직을 변경하면 안 됨. QC/폴더와 tests/폴더를 새로 만들거나 보강하는 것만 허용.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 시스템 1: 하드웨어 QC (C++ — 기기 위에서 실행)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

목적: 기기가 부팅된 후 loop()에서 매 사이클/주기적으로 하드웨어 상태를 자동 점검하여
      시리얼 모니터에 [FAIL] / [WARN] 로그를 남김. 현장에서 즉시 문제를 발견하는 용도.

### 구조
QC/
├── QC_Types.h      ← QCLevel(PASS/WARN/FAIL), QCResult(구조체), IQCRule(인터페이스)
├── QC_Engine.h     ← 싱글턴 엔진. addRule(), tick(), FAST/SLOW 분리
└── QC_Rules.h      ← 실제 규칙 클래스들

### 엔진 동작
- setup()에서 QCEngine::getInstance().begin(1000) + addRule()
- loop()에서 QCEngine::getInstance().tick()
- FAST 규칙: 매 tick마다 실행 (isFastCheck() == true)
- SLOW 규칙: begin(ms)에 설정된 주기마다 실행

### 규칙 작성법 (IQCRule 상속)
각 규칙 클래스는 반드시 아래를 구현:
- getId()        → 고유 Rule ID (예: "HW_RFID_01")
- getName()      → 사람 읽기용 이름
- isFastCheck()  → true(매 tick) / false(주기적)
- check()        → QCResult 반환. PASS면 빈 QCResult(), 이슈면 level/id/what/criterion/value/fix 채움

### 로그 포맷
[FAIL] [RULE_ID] what value (Limit: criterion) -> Fix: fix
[WARN] [RULE_ID] what value (Limit: criterion) -> Fix: fix

### 필수 규칙 카테고리 (레포 분석 후 해당하는 것 모두 구현)

#### 시스템
- SYS_MEM_xx: Free Heap 부족 감지 (WARN < 20KB, FAIL < 10KB)
- SYS_RST_xx: 마지막 리셋 원인 (Brownout→FAIL, WDT/Panic→WARN)

#### 네트워크
- NET_WIFI_xx: WiFi 연결 상태 (FAST), RSSI 신호 강도 (SLOW)
- NET_CONN_xx: 서버 연결 상태 (해당 시)

#### 하드웨어 — ★ 핵심: 하드웨어 오류를 잡아내는 규칙 ★
- HW_PIN_xx: 정적 핀 충돌 감지 (동일 GPIO에 2개 이상 기능 매핑 → FAIL)
- HW_GPIO_xx: GPIO 특성 불일치 (input-only 핀에 pull-up 필요 기능 매핑 → FAIL)
- HW_RFID_xx: RFID/NFC 초기화 상태 + 런타임 ACK heartbeat (SPI 링크 끊김 감지)
- HW_ENC_xx: 엔코더/센서 값 범위 이탈 (노이즈/단선 감지)
- HW_MOT_xx: 모터/액추에이터 동작 중 전원 강하 감지
- HW_UART_xx: UART 디스플레이 통신 상태 (해당 시)
- HW_I2C_xx: I2C 센서 응답 확인 (해당 시)
- HW_SPI_xx: SPI 디바이스 통신 확인 (해당 시)

#### 로직
- LOGIC_STATE_xx: 서버/로컬 상태값 유효성 검증
- LOGIC_FSM_xx: FSM 상태와 서버 상태 정합성 확인

### 규칙 도출 절차
1. .ino, .h 파일 전체를 읽고 핀 매핑, 통신 프로토콜, FSM, 센서 목록을 추출
2. Library_and_pin.h (또는 동급 파일)에서 핀 번호 + GPIO 특성 확인
3. 각 하드웨어 모듈별로 "이 모듈이 고장나면 어떤 증상이 나타나는가?" 역추적
4. 증상을 감지할 수 있는 QC 규칙으로 변환
5. 핀 충돌, GPIO 특성 불일치는 정적 분석으로 즉시 FAIL 판정

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 시스템 2: Python 테스트 하네스 (pytest — PC에서 실행)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

목적: 하드웨어 없이 PC에서 FSM 로직, 상태 전이, 멱등성, 네트워크 장애 대응,
      QC 규칙 판정 로직을 검증. 코드 변경 시 기존 동작이 깨지지 않는지 확인.

### 구조
tests/
├── __init__.py
├── conftest.py         ← pytest fixtures (model, server, shim)
├── itembox_model.py    ← 펌웨어 FSM의 순수 파이썬 1:1 모델
├── fake_server.py      ← 서버 메시지 시뮬레이터
├── net_shim.py         ← 네트워크 지연/드랍/중복 시뮬레이터
├── sensor_gen.py       ← 센서값 생성기 (spike, noise, drift)
├── test_fsm_transitions.py
├── test_idempotency.py
├── test_network_faults.py
├── test_qc_rules.py
├── test_reset_recovery.py
└── test_sensor_anomaly.py

### Python 모델 작성법
1. .ino/.h의 FSM enum을 Python Enum으로 1:1 복제
2. 하드웨어 함수(BoxOpen, NeoPixelOn 등)는 호출 기록(HWCall)만 남김
3. 서버 통신(Send)은 outbox 큐에 메시지 추가
4. DataChanged() / 각 상태 핸들러를 메서드로 재현
5. run_qc_check()에 C++ QC_Rules.h의 모든 규칙을 파이썬으로 재구현
   — C++ 코드와 동일한 임계값, 동일한 Rule ID 사용
6. 결정적(deterministic) 실행 보장 — random.seed 고정

### 테스트 카테고리

#### FSM 전이 (test_fsm_transitions.py)
- Happy Path: 전체 게임 흐름을 순서대로 검증
- Invalid Transition: 잘못된 상태 전이 시도 → 무시/올바른 처리 확인
- Reset During Game: 게임 중 리셋 → 초기 상태 복원 확인

#### 멱등성 (test_idempotency.py)
- 동일 명령 중복 수신 시 하드웨어 함수가 1번만 호출되는지 확인
- 각 주요 상태(setting, open, used)에 대해 테스트

#### 네트워크 장애 (test_network_faults.py)
- 지연(delay): N ms 지연 후에도 정상 동작 확인
- 드랍(drop): 100% 드랍 시 상태 불변, 해제 후 복구 확인
- 중복(duplicate): 메시지 2회 도달 시 멱등성 확인

#### QC 규칙 판정 (test_qc_rules.py)
- C++ QC_Rules.h의 모든 규칙을 파이썬 모델로 테스트
- 각 규칙의 정상/WARN/FAIL 경계값 테스트
- ★ 하드웨어 규칙(HW_*)도 반드시 포함 ★

#### 리셋/복구 (test_reset_recovery.py)
- 게임 중 reset() → 초기 상태 복원 확인
- WiFi 끊김 → 재연결 후 QC 에러 해소 확인

#### 센서 이상 (test_sensor_anomaly.py)
- 센서값 글리치(spike) → QC WARN 발생 → 복구 후 PASS
- 미등록/유령 태그 → 올바른 로그 + 상태 불변

### conftest.py 필수 fixture
- model: ItemBoxModel 인스턴스 (boot() 호출 완료)
- server: FakeServer (model과 연결)
- shim_server: NetShim이 적용된 server (네트워크 장애 테스트용)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 실행 순서
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1단계: 레포 전체 분석
   → 핀 맵, FSM, 통신 프로토콜, 센서, 액추에이터 목록 추출

2단계: QC/ 폴더 생성 (C++)
   → QC_Types.h → QC_Engine.h → QC_Rules.h 순서로 작성
   → 메인 .ino에 #include, begin(), addRule(), tick() 추가

3단계: tests/ 폴더 생성 (Python)
   → itembox_model.py (FSM + QC 모델) → fake_server.py → conftest.py
   → 테스트 파일 작성

4단계: 검증
   → python -m pytest tests/ -v → 전체 PASS 확인

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 핵심 원칙
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1. 기존 .ino 로직 절대 변경 금지 (QC/와 tests/만 추가)
2. C++ Rule ID와 Python Rule ID는 반드시 동일해야 함
3. C++ 임계값과 Python 임계값은 반드시 동일해야 함
4. 하드웨어 오류 감지가 최우선 — 핀 충돌, SPI 링크 끊김, 전원 강하 등
   → "이 모듈이 고장나면 어떤 증상?" 역추적 방식으로 규칙 도출
5. Python 테스트는 결정적이어야 함 (flaky test 금지)
6. QC 로그는 현장에서 즉시 판독 가능해야 함 ([FAIL] [ID] ... → Fix: ...)
```

---

## 참고 구현 (updated_itembox)

이 프롬프트로 만들어진 실제 구현 사례:
- `QC/QC_Types.h` — 3단계 레벨(PASS/WARN/FAIL) + 구조체 + 인터페이스
- `QC/QC_Engine.h` — 싱글톤 엔진, FAST/SLOW 분리, tick() 기반
- `QC/QC_Rules.h` — 11개 규칙 (NET×2, SYS×2, HW×5, LOGIC×2)
- `tests/itembox_model.py` — 700줄 FSM 모델 + 11개 QC 규칙 재구현
- `tests/test_qc_rules.py` — 9개 QC 판정 테스트
- 총 24개 pytest 테스트, 전부 PASS
