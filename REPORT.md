# 아이템박스 READY 시 모터 미작동 버그 분석 보고서

- **작성일:** 2026-03-19
- **대상 펌웨어:** updated_itembox (ESP32)
- **수정 파일:** `motor.ino`
- **수정 커밋:** `5b440d2`

---

## 1. 문제 현상

| 동작 | 결과 |
|------|------|
| 보드 리셋 → `MotorInit()` | 모터 정상 작동 (박스 닫힘) ✓ |
| SETTING 명령 → `BoxOpen()` | 모터 정상 작동 (박스 열림) ✓ |
| READY 명령 → `BoxClose()` | **모터 완전 무반응** (소리·진동 없음) ✗ |

- `BOX Close` / `BOX Closed` Serial 로그는 정상 출력됨
- 즉, 코드 실행 자체는 정상이나 **하드웨어 제어 신호가 H브리지에 도달하지 못함**

---

## 2. 디버깅 과정

### 2-1. 초기 가설들 (모두 기각)

#### 가설 1: H브리지 열보호
- 근거: BoxOpen(4초 풀파워) 이후 BoxClose 실패
- **기각 이유:** 보드 리셋 시 즉시 BoxClose가 성공함. 열보호는 하드웨어 상태이므로 보드 리셋으로 복구되지 않음

#### 가설 2: MotorStop이 PWM을 0으로 리셋하지 않아 H브리지가 방향 전환에 미반응
- 근거: `MotorStop()`에 `ledcWrite(MOTOR_PWMA_PIN, 0)` 미존재 추정
- **기각 이유:** 코드 확인 결과 이미 존재함

### 2-2. Serial 디버그 삽입

`BoxClose()` 내부에 다음 디버그 코드 추가:

```cpp
Serial.print("LEDC duty before: ");
Serial.println(ledcRead(MOTOR_PWMA_PIN));

ledcWrite(MOTOR_PWMA_PIN, MotorMAX_DUTY_CYCLE - 1);

Serial.print("LEDC duty after: ");
Serial.println(ledcRead(MOTOR_PWMA_PIN));

Serial.print("INA1: "); Serial.println(digitalRead(MOTOR_INA1_PIN));
Serial.print("INA2: "); Serial.println(digitalRead(MOTOR_INA2_PIN));
```

### 2-3. 디버그 결과 (READY 호출 시)

```
LEDC duty before: 0
LEDC duty after:  0     ← ledcWrite(MAX) 했는데도 0 그대로
INA1: 0
INA2: 0                 ← digitalWrite(HIGH) 했는데도 0 그대로
```

**핵심 발견:**
1. `ledcWrite(MOTOR_PWMA_PIN, MAX)` 호출 후에도 `ledcRead`가 0 반환 → LEDC 채널이 핀에서 분리된 상태
2. `digitalWrite(MOTOR_INA2_PIN, HIGH)` 호출 후에도 `digitalRead`가 0 반환 → GPIO4가 OUTPUT 모드를 잃은 상태

### 2-4. ledcAttach 추가 후 재디버그

`BoxClose()` 앞에 `ledcAttach(MOTOR_PWMA_PIN, MotorFreq, MotorResolution)` 추가 후 재측정:

```
첫 번째 ACTIVATE → BoxClose:
  LEDC duty: 0     ← 여전히 실패
  INA2: 0

두 번째 ACTIVATE → BoxClose (4초 후):
  LEDC duty: 254   ← LEDC는 복구됨
  INA2: 0          ← INA2는 여전히 0
```

- LEDC 문제와 INA2 문제가 **독립적인 두 가지 버그**임을 확인
- LEDC=254인데 INA2=0이라 두 번째 ACTIVATE에서도 모터가 무반응

---

## 3. 원인 분석

### 원인 1: `ledcWrite(0)`이 LEDC 채널을 핀에서 분리함

#### 동작 원리

ESP32 Arduino Core에서 LEDC(LED Control) 주변기기는 내부 채널을 GPIO 매트릭스를 통해 물리적 핀에 연결한다. `ledcAttach(pin, freq, res)` 호출 시 채널이 핀에 라우팅되며, 이후 `ledcWrite(pin, duty)`로 PWM 출력을 제어한다.

#### 문제 동작

일부 ESP32 Arduino Core 버전에서 `ledcWrite(pin, 0)`을 호출하면, duty를 0으로 설정하는 것에 더해 **핀을 LEDC 채널에서 분리하고 일반 GPIO 모드로 전환**하는 최적화가 발생한다.

```
ledcWrite(MOTOR_PWMA_PIN, 0)  →  LEDC 채널 분리 (핀이 GPIO 모드로 전환)
ledcWrite(MOTOR_PWMA_PIN, MAX)  →  LEDC 레지스터만 업데이트, 핀은 여전히 GPIO 모드
                                    → 물리적 핀 전압 변화 없음 → 모터 무반응
```

#### 발생 위치

```cpp
// motor.ino
void MotorStop() {
    digitalWrite(MOTOR_INA1_PIN, LOW);
    digitalWrite(MOTOR_INA2_PIN, LOW);
    ledcWrite(MOTOR_PWMA_PIN, 0);  ← 이 호출이 채널 분리 유발
}
```

#### 왜 부팅 시에는 정상인가?

`MotorInit()`에서의 순서:
```cpp
ledcAttach(MOTOR_PWMA_PIN, ...);  // (1) 채널 연결
ledcWrite(MOTOR_PWMA_PIN, 0);     // (2) 채널 분리 발생
BoxClose();                        // (3) BoxClose 내부의 ledcAttach가 재연결
  └─ ledcAttach(MOTOR_PWMA_PIN, ...)  // ← 재연결 성공 (이전 분리 후 즉시 재연결)
  └─ ledcWrite(MAX)                   // ← 정상 동작
```

`MotorInit()`에서는 `ledcAttach`가 두 번 호출되는 구조여서 (한 번은 init에서, 한 번은 BoxClose에서) 정상 동작함.

이후 호출 시에는 `MotorStop()` → `ledcWrite(0)` (분리) → 이후 `BoxClose()`에서 `ledcAttach` 한 번 → 경우에 따라 첫 번째 호출에서 재연결이 불완전해 LEDC duty가 0으로 잔류.

---

### 원인 2: WiFi 전송 후 GPIO4(MOTOR_INA2_PIN)가 OUTPUT 모드를 잃음

#### 문제 동작

`ReadyFunc()`에서 `has2wifi.Send()`를 3회 호출 (HTTP GET 요청) 한 뒤 `BoxClose()`가 실행될 때, `MOTOR_INA2_PIN`(GPIO4)이 `OUTPUT` 모드에서 다른 모드로 전환되어 있음.

```
has2wifi.Send() x3  →  ESP32 WiFi 스택 동작 (HTTP 전송)
                    →  내부적으로 GPIO4 모드 변경 (OUTPUT → INPUT 또는 기타)

BoxClose()
  └─ digitalWrite(MOTOR_INA2_PIN, HIGH)
       →  OUTPUT 모드가 아니므로 핀이 HIGH로 구동되지 않음
       →  외부 풀다운 또는 H브리지 입력 저항에 의해 핀이 LOW로 읽힘
```

#### H브리지에 미치는 영향

| INA1 | INA2 (실제) | 모터 동작 |
|------|------------|----------|
| LOW | HIGH (의도) | 닫힘 방향 회전 |
| LOW | LOW (실제) | **브레이크 모드 — 정지** |

모터가 완전히 무음인 이유: PWM 신호가 있어도 INA1=LOW, INA2=LOW이면 H브리지 출력이 차단됨.

#### 왜 BoxOpen은 정상인가?

BoxOpen의 방향은 `INA1=HIGH, INA2=LOW`이다. INA2가 LOW로 고착된 상태는 BoxOpen에서 요구하는 값과 **일치**하므로 정상 동작한다.

```
BoxOpen:  INA1=HIGH, INA2=LOW  → INA2 고착 상태와 일치 → 정상 ✓
BoxClose: INA1=LOW, INA2=HIGH  → INA2가 HIGH로 안 올라감 → 브레이크 ✗
```

#### 왜 부팅 시에는 정상인가?

`MotorInit()` 직후에는 `has2wifi.Send()`가 아직 호출되지 않아 GPIO4가 `OUTPUT` 모드를 유지하고 있음. `MotorInit()` 내에서 `pinMode(MOTOR_INA2_PIN, OUTPUT)`이 설정되고, 이후 WiFi 전송 없이 바로 `BoxClose()`가 실행되므로 정상 동작.

---

## 4. 해결 방법

### 수정 코드 (`motor.ino`)

```cpp
void BoxClose()
{
    Serial.println("BOX Close");
    pinMode(MOTOR_INA1_PIN, OUTPUT);     // WiFi 이후 핀 모드 복구
    pinMode(MOTOR_INA2_PIN, OUTPUT);     // WiFi 이후 핀 모드 복구
    ledcAttach(MOTOR_PWMA_PIN, MotorFreq, MotorResolution);  // LEDC 채널 재연결
    ledcWrite(MOTOR_PWMA_PIN, MotorMAX_DUTY_CYCLE - 1);
    digitalWrite(MOTOR_INA1_PIN, LOW);
    digitalWrite(MOTOR_INA2_PIN, HIGH);
    delay(4000);
    MotorStop();
    Serial.println("BOX Closed");
}

void BoxOpen()
{
    Serial.println("BOX Open");
    pinMode(MOTOR_INA1_PIN, OUTPUT);     // WiFi 이후 핀 모드 복구
    pinMode(MOTOR_INA2_PIN, OUTPUT);     // WiFi 이후 핀 모드 복구
    ledcAttach(MOTOR_PWMA_PIN, MotorFreq, MotorResolution);  // LEDC 채널 재연결
    ledcWrite(MOTOR_PWMA_PIN, MotorMAX_DUTY_CYCLE - 1);
    digitalWrite(MOTOR_INA1_PIN, HIGH);
    digitalWrite(MOTOR_INA2_PIN, LOW);
    delay(4000);
    MotorStop();
    Serial.println("BOX Opened");
}

void MotorStop()
{
    Serial.println("모터 스탑");
    digitalWrite(MOTOR_INA1_PIN, LOW);
    digitalWrite(MOTOR_INA2_PIN, LOW);
    // ledcWrite(0) 제거 — LEDC 채널 연결 유지
    // 브레이크 모드는 INA1=LOW, INA2=LOW만으로 충분
}
```

### 수정 포인트 요약

| 수정 | 대상 버그 | 설명 |
|------|-----------|------|
| `MotorStop()`에서 `ledcWrite(0)` 제거 | 원인 1 | LEDC 채널 분리 방지. 모터 정지는 INA1/INA2 LOW(브레이크 모드)로 충분 |
| `BoxClose()`/`BoxOpen()` 앞에 `ledcAttach()` 추가 | 원인 1 | 혹시 분리되었더라도 매번 재연결 보장 |
| `BoxClose()`/`BoxOpen()` 앞에 `pinMode(OUTPUT)` 추가 | 원인 2 | WiFi 전송 후 변경된 GPIO 모드를 OUTPUT으로 복구 |

---

## 5. 결론

본 버그는 소프트웨어 로직 오류가 아닌 **ESP32 하드웨어 주변기기(LEDC, GPIO 매트릭스)와 WiFi 스택 간의 부작용**으로 인해 발생하였다.

- `ledcWrite(0)`의 LEDC 채널 분리 부작용은 ESP32 Arduino Core 내부 구현에 기인하며, duty=0 설정 시 핀을 GPIO 모드로 전환하는 최적화가 원인이다.
- WiFi 전송(`has2wifi.Send()`) 이후 GPIO4 모드 변경은 ESP32 WiFi 스택이 내부 동작 중 일부 GPIO 설정을 변경하는 것으로 추정된다.

두 문제 모두 `BoxOpen()`/`BoxClose()` 호출 시점에 `pinMode(OUTPUT)`과 `ledcAttach()`를 명시적으로 재설정하는 것으로 해결되었으며, 이후 READY/SETTING/ACTIVATE 전 상태에서 모두 정상 동작이 확인되었다.
