# ItemBox (Random Puzzle Add)

ESP32 기반 방탈출 아이템박스 펌웨어입니다.
RFID 태그 인식 → 엔코더 퍼즐 풀기 → 박스 개방의 흐름으로 동작하며, WiFi를 통해 서버와 상태를 동기화합니다.

---

## 하드웨어 구성

| 부품 | 역할 |
|------|------|
| ESP32 | 메인 컨트롤러 |
| Adafruit PN532 x2 | 외부 / 내부 RFID 태그 인식 |
| Adafruit NeoPixel x3 | 상태 표시 LED (PN532용 28개, 인코더용 24개, 내부용 24개) |
| 로터리 엔코더 | 퍼즐 입력 |
| 리니어 모터 | 박스 개폐 |
| 진동 모터 | 퍼즐 힌트 (정답에 가까울수록 강하게 진동) |
| Nextion 디스플레이 | 게임 UI 출력 |

### 핀 배치

| 핀 | 기능 |
|----|------|
| 25 | PN532 NeoPixel |
| 26 | Encoder NeoPixel |
| 27 | Inner NeoPixel |
| 5 / 21 | PN532 SS1 / SS2 |
| 18 / 19 / 23 | PN532 SCK / MISO / MOSI |
| 32 / 4 / 22 | Motor INA1 / INA2 / PWM |
| 13 / 15 / 34 | Encoder A / B / Button |
| 14 | 진동모터 PWM |
| 12 | 진동모터 Answer |
| 33 / 39 | Nextion TX / RX |

---

## 소프트웨어 구조

```
itembox_random_add.ino   - setup() / loop()
itembox_random_add.h     - 전역 변수 및 객체 선언
Library_and_pin.h        - 라이브러리 include 및 핀 정의
Wifi.ino                 - WiFi 상태 수신 및 게임 상태 전환 함수
neopixel.ino             - NeoPixel 초기화 및 제어
motor.ino                - 리니어모터 / 진동모터 제어
rfid.ino                 - RFID 태그 인식 및 퍼즐 흐름
encoder.ino              - 로터리 엔코더 인터럽트 처리
Game_system.ino          - 퍼즐 로직 (엔코더 정답 판정)
timer.ino                - SimpleTimer 기반 주기 함수 관리
nextion.ino              - Nextion 디스플레이 통신
```

---

## 게임 흐름

```
setting → ready → activate → (RFID 태그) → 퍼즐(엔코더) → open → (아이템 수령)
```

| 서버 상태 | 동작 |
|-----------|------|
| `setting` | 초기화, 박스 열기, 전체 WHITE LED |
| `ready` | 퍼즐 정답 랜덤 생성, 박스 닫기, 배터리팩 분배 전송 |
| `activate` | 게임 시작, YELLOW LED, 엔코더 퍼즐 대기 |
| `open` | 박스 개방, YELLOW 점멸, 내부 RFID 대기 |
| `used` | 아이템 수령 처리, RED LED |
| `repaired_all` | 전체 수리 완료, BLUE LED, 박스 개방 |
| `player_win` | 플레이어 승리 |
| `player_lose` | 플레이어 패배 |

---

## 퍼즐 정답 랜덤 생성 (ReadyFunc)

- 정답 개수: 3개
- 1번 정답: 10 ~ 23 고정 범위
- 2~3번 정답: 1 ~ 95 범위
- 정답 간격: 최소 10칸, 최대 30칸
- 생성된 정답은 WiFi로 서버에 전송됨

---

## 사용 라이브러리

- `Adafruit_NeoPixel`
- `Adafruit_PN532`
- `ArduinoJson`
- `HAS2_Wifi`
- `SimpleTimer`
- `Nextion` (ITEADLIB)

---

## 빌드 환경

- Board: ESP32 Dev Module
- Arduino IDE
- ESP32 Arduino Core v3.x 이상 (`ledcAttach` API 사용)
