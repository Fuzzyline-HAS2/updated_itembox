# 🎁 아이템박스 (ItemBox) 펌웨어

> 방탈출 게임용 **아이템박스** 장치의 ESP32 펌웨어입니다.  
> 엔코더 퍼즐을 풀면 상자가 열리고, 플레이어가 RFID 태그로 배터리팩을 가져갈 수 있습니다.

---

## 📋 목차
- [개요](#-개요)
- [하드웨어 구성](#-하드웨어-구성)
- [파일 구조](#-파일-구조)
- [FSM 상태 전이](#-fsm-상태-전이)
- [게임 플로우](#-게임-플로우)
- [QC 시스템](#-qc-시스템)
- [설치 및 업로드](#-설치-및-업로드)

---

## 🎯 개요

아이템박스는 방탈출에서 **에너지원(배터리팩)** 을 보관하는 물리적 장치입니다.

**작동 원리:**
1. 관리자가 게임을 시작하면 박스가 닫히고 대기
2. 플레이어가 외부 RFID를 태그하면 엔코더 퍼즐이 시작
3. 엔코더를 돌려 정답을 맞추면 박스가 열림
4. 플레이어가 내부 RFID를 태그하면 배터리팩 획득 완료

---

## 🔧 하드웨어 구성

| 부품 | 역할 | 핀 |
|---|---|---|
| **ESP32** | 메인 MCU | — |
| **PN532 × 2** | 외부/내부 RFID 리더 | SPI (SCK:18, MISO:19, MOSI:23, SS1:5, SS2:21) |
| **로터리 엔코더** | 퍼즐 입력 장치 | A:13, B:15, SW:34 |
| **네오픽셀 × 3** | 상태 표시 LED (외부RFID/엔코더/내부RFID) | 25, 26, 27 |
| **리니어 모터** | 박스 개폐 | INA1:32, INA2:4, PWM:22 |
| **진동 모터** | 엔코더 힌트 피드백 | 12, 14 |
| **Nextion 디스플레이** | 화면 표시 | RX:39, TX:33 |
| **박스 스위치** | 박스 열림/닫힘 감지 | 36 |

---

## 📁 파일 구조

```
updated_itembox/
├── updated_itembox.ino   # 메인 스케치 (setup/loop)
├── updated_itembox.h     # 전역 변수 및 함수 선언
├── Library_and_pin.h        # 라이브러리 & 핀 정의
├── ItemBoxState.h           # FSM 상태 정의 및 전이 함수
│
├── Wifi.ino                 # WiFi 데이터 수신 + 상태별 핸들러
├── rfid.ino                 # 내부/외부 RFID 처리
├── Game_system.ino          # 엔코더 퍼즐 로직
├── motor.ino                # 리니어 모터(박스 개폐) + 진동 모터
├── encoder.ino              # 로터리 엔코더 입력
├── neopixel.ino             # 네오픽셀 LED 제어
├── nextion.ino              # Nextion 디스플레이 통신
├── timer.ino                # SimpleTimer 기반 주기 실행
│
└── QC/                      # QC (Quality Check) 시스템
    ├── QC_Engine.h           # QC 엔진 (싱글톤, 룰 관리)
    ├── QC_Rules.h            # 7개 QC 룰 정의
    └── QC_Types.h            # QC 타입 (IQCRule, QCResult 등)
```

---

## 🔄 FSM 상태 전이

아이템박스 내부에는 12개 상태를 가진 **FSM(Finite State Machine)** 이 있습니다.  
서버의 `game_state`/`device_state`와는 **별개로** 장치 내부 상태를 추적합니다.

### 상태 목록

| 상태 | 설명 | LED 색상 |
|---|---|---|
| `BOOT` | 초기 부팅 중 | — |
| `SETTING` | 세팅 모드 (박스 열림) | ⬜ 흰색 |
| `READY` | 게임 대기 (박스 닫힘) | 🟥 빨간색 |
| `ACTIVATE` | 외부 RFID 태그 대기 | 🟨 노란색 |
| `PUZZLE` | 엔코더 퍼즐 진행 중 | 🟦 파란색 |
| `QUIZ_COMPLETE` | 퍼즐 완료, 확인 태그 대기 | 🟨 점멸 |
| `OPEN` | 박스 열림, 아이템 수거 대기 | 🟦 파란색 |
| `USED` | 아이템 수거 완료 | 🟥 빨간색 |
| `CLOSE` | 관리자 강제 닫기 | — |
| `REPAIRED_ALL` | 전체 수리 완료 | 🟦 파란색 |
| `PLAYER_WIN` | 플레이어 승리 | 🟦 파란색 |
| `PLAYER_LOSE` | 플레이어 패배 | 🟥 빨간색 |

### 상태 전이 다이어그램

```
                    ┌──────────────────────────────────────────────────┐
                    │              관리자 대시보드                       │
                    └──────┬──────────┬─────────────┬─────────────────┘
                           │          │             │
                    game_state   device_state   device_state
                    ="setting"   ="repaired_all" ="player_win/lose"
                           │          │             │
                           ▼          ▼             ▼
   ┌──────┐    ┌─────────┐    ┌───────┐    ┌──────────┐    ┌────────────┐
   │ BOOT │───▶│ SETTING │───▶│ READY │    │REPAIRED  │    │ PLAYER_WIN │
   └──────┘    └─────────┘    └───┬───┘    │  _ALL    │    │ PLAYER_LOSE│
                  ▲               │        └──────────┘    └────────────┘
                  │          game_state
                  │          ="activate"
                  │               │
                  │               ▼
                  │         ┌──────────┐    외부 RFID    ┌────────┐
                  │         │ ACTIVATE │───────────────▶│ PUZZLE │
                  │         └──────────┘    태그         └───┬────┘
                  │                                         │
                  │                                    정답 3개 완료
                  │                                         │
                  │                                         ▼
                  │                                ┌───────────────┐
                  │                                │ QUIZ_COMPLETE │
                  │                                └───────┬───────┘
                  │                                        │
                  │                                   외부 RFID
                  │                                    태그 (확인)
                  │                                        │
                  │                                        ▼
                  │                                   ┌────────┐
                  │                                   │  OPEN  │
                  │                                   └───┬────┘
                  │                                       │
                  │                                  내부 RFID
                  │                                   태그 (수거)
                  │                                       │
                  │                                       ▼
                  │                                   ┌────────┐
                  └───────────────────────────────────│  USED  │
                         다음 게임 시작                  └────────┘
```

### Serial Monitor 로그 예시

```
[FSM] BOOT -> SETTING
[FSM] SETTING -> READY
[FSM] READY -> ACTIVATE
[FSM] ACTIVATE -> PUZZLE
[FSM] PUZZLE -> QUIZ_COMPLETE
[FSM] QUIZ_COMPLETE -> OPEN
[FSM] OPEN -> USED
```

---

## 🎮 게임 플로우

```
1. 관리자가 "setting" 전송
   → 박스 열림, 전체 LED 흰색, 초기화

2. 관리자가 "ready" 전송
   → 배터리팩 랜덤 배분, 박스 닫힘, 전체 LED 빨간색

3. 관리자가 "activate" 전송
   → 전체 LED 노란색, 외부 RFID 대기

4. 플레이어가 외부 RFID 태그
   → 엔코더 퍼즐 시작, 전체 LED 파란색
   → 진동 모터로 정답 힌트 제공 (가까울수록 강한 진동)

5. 엔코더로 정답 3개 맞추면
   → 퀴즈 완료! 외부 RFID 태그로 확인

6. 확인 태그 후
   → 박스 열림, 내부 LED 노란색 점멸

7. 플레이어가 내부 RFID 태그 (배터리팩 수거)
   → 배터리팩 전달, 전체 LED 빨간색, 대기 상태
```

---

## 🔍 QC 시스템

운영 중 장치 상태를 자동 점검하는 **QC(Quality Check)** 시스템이 내장되어 있습니다.

| ID | 룰 이름 | 점검 내용 |
|---|---|---|
| `NET_WIFI_00` | WiFi Connection | WiFi 연결 상태 |
| `NET_WIFI_01` | WiFi Signal | RSSI 신호 세기 (-75dBm 미만 경고) |
| `SYS_MEM_01` | Heap Memory | 잔여 힙 메모리 (20KB 미만 경고) |
| `LOGIC_STATE_01` | Game State | game/device/manage_state 유효성 |
| `HW_RFID_01` | Dual RFID Init | Inner/Outer PN532 초기화 상태 |
| `HW_ENC_01` | Encoder Range | 엔코더 값 범위 (0~380) |
| `LOGIC_FSM_01` | FSM Consistency | 서버 state ↔ 로컬 FSM 일치 여부 |

QC 결과는 HTTP GET `/api/qc-report`로 조회 가능합니다.

---

## 🚀 설치 및 업로드

### 필수 라이브러리
- `Adafruit_NeoPixel`
- `Adafruit_PN532`
- `ArduinoJson`
- `SimpleTimer`
- `Nextion` (ITEADLIB)
- `HAS2_Wifi` (자체 라이브러리)

### Arduino IDE 설정
| 항목 | 값 |
|---|---|
| **보드** | ESP32 Dev Module |
| **파티션 스킴** | Minimal SPIFFS (1.9MB APP with OTA) |
| **시리얼 속도** | 115200 |

### 업로드 방법
1. Arduino IDE에서 `updated_itembox.ino` 열기
2. 보드/포트 선택
3. `Ctrl+U` 또는 `스케치 → 업로드`
4. Serial Monitor에서 `QC System Started` 확인

---

## 📡 서버 통신

서버와 WiFi로 JSON 데이터를 주고받습니다.

**수신 데이터 (서버 → 아이템박스):**
```json
{
  "device_name": "itembox",
  "device_type": "itembox",
  "game_state": "ready",
  "device_state": "activate",
  "manage_state": null,
  "battery_pack": "1",
  "exp_pack": "50",
  "light_mode": "day",
  "game_mode": "normal"
}
```

**송신 예시 (아이템박스 → 서버):**
```
device_state = "open"     ← 퍼즐 완료 시
device_state = "used"     ← 아이템 수거 시
```
