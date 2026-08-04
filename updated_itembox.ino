/**
 * @file Done_ItemBox_code.ino
 * @author 김병준 (you@domain.com)
 * @brief
 * @version 1.0
 * @date 2022-11-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#define FIRMWARE_VER 33
#define PARTITION_VER 8
#include "updated_itembox.h"
#include "esp_system.h"

void setup()
{
    Serial.begin(115200);
    has2wifi.SetDebugPrint(&Serial); // 라이브러리 로그를 DebugSerial(UART0+텔넷)로 보냄. 전역 Serial은 begin되지 않아 출력이 버려짐

    // has2wifi.Setup("badland");
    has2wifi.Setup("city");
    // has2wifi.Setup("badland_ruins","Code3824@");
    has2wifi.Send((String)(const char*)my["device_name"], "esp_version", String(FIRMWARE_VER)); // 부팅 시 현재 펌웨어 버전을 서버 device.esp_version에 보고 (OTA 재부팅 후에도 자동 갱신)
    TelnetInit();
    BleAdvertiserInit();
    Serial.println("MAC: " + WiFi.macAddress());
    ota.setLogStream(Serial);
    ota.setOnSuccess([]() {
        for (int i = 0; i < 5; i++) {
            AllNeoOn(RED);
            delay(300);
            AllNeoOn(BLACK);
            delay(300);
        }
        has2wifi.Send((String)(const char*)my["device_name"], "device_state", "setting");
    });
    ota.setOnSkip([]() {
        has2wifi.Send((String)(const char*)my["device_name"], "device_state", "setting");
    });
    ota.setPartitionUpdate(
        "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_itembox/third_store/partitions.bin",
        "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_itembox/third_store/partitions.sig",
        "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_itembox/third_store/partition_version.txt",
        PARTITION_VER
    );
    NeopixelInit();
    RfidInit();
    MotorInit();
    EncoderInit();
    NextionInit();
    TimerInit();
    DataChanged();
}
void loop()
{
    if (boxMotorRunning) {
        if (boxClosing) {
            if (digitalRead(BOXSWITCH_PIN) == HIGH) {  // 닫힘 감지 → 모터 정지
                MotorStop();
                boxMotorRunning = false;
                Serial.println("BOX Closed");
            }
        } else {
            if (millis() - motorStartTime >= motorOpenDuration) {  // 타이머 만료 → 모터 정지
                MotorStop();
                boxMotorRunning = false;
                Serial.println("BOX Opened");
                if (pendingOpenScreen) {
                    delay(motorSettleDelay);  // 모터 정지 후 전원 레일 안정화 대기 → 이어지는 WiFi 송신 전류가 모터 전류와 겹치지 않게 (brownout 방지)
                    BatteryPackSend();
                    sendCommand("page pgItemOpen");
                    SendLanguage();
                    ExpSend();
                    if ((String)(const char*)shift_machine["selected_language"] != "EN")
                        sendCommand("wQuizSolved.en=1");
                    else
                        sendCommand("wEQuizSolved.en=1");
                    pendingOpenScreen = false;
                    // 화면 전환을 끝낸 뒤 서버 보고. 모터 정지·안정화 이후라 WiFi 전류 피크가 모터와 겹치지 않음.
                    // 내부 태그가 먼저 처리돼 used가 됐다면 open으로 덮어쓰지 않도록 가드.
                    if (!itemBoxUsed)
                        has2wifi.Send((String)(const char*)my["device_name"], "device_state", "open");
                    // 박스 오픈 완료 후 HTTP 폴링 타이머 재개.
                    WifiTimer.deleteTimer(wifiTimerId);
                    wifiTimerId = WifiTimer.setInterval(wifiTime, WifiIntervalFunc);
                    // 모터 정지 + 안정화 + open 보고까지 끝난 뒤에야 내부 태그 처리 활성화.
                    // (PuzzleSolved에서 WaitFunc로 막아둔 것을 여기서 RfidLoopInner로 전환)
                    if (!itemBoxUsed) {
                        ptrCurrentMode = RfidLoopInner;
                        ptrRfidMode = ItemTook;
                    }
                }
                // 서버 open 경로: 모터 정지 후 전원 안정화 대기 → 내부 태그 활성화.
                // (Wifi.ino에서 WaitFunc로 막아둔 것을 여기서 RfidLoopInner로 전환, 이후 ItemTook WiFi 송신이 모터 전류와 겹치지 않음)
                if (pendingInnerEnable) {
                    delay(motorSettleDelay);
                    pendingInnerEnable = false;
                    if (!itemBoxUsed) {
                        ptrCurrentMode = RfidLoopInner;
                        ptrRfidMode = ItemTook;
                    }
                }
            }
        }
    }
    TelnetRun();
    BleAdvertiserMaintain();
    ptrCurrentMode();
    WifiTimer.run();
    GameTimer.run();
    BlinkTimer.run();
}
