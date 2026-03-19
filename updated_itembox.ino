 ;/**
 * @file Done_ItemBox_code.ino
 * @author 김병준 (you@domain.com)
 * @brief
 * @version 1.0
 * @date 2022-11-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "updated_itembox.h"
#include "esp_system.h"

void setup()
{
    Serial.begin(115200);

    // 리셋 원인 확인 - NeoPixel로 시각적 표시
    // 먼저 NeoPixel 최소 초기화
    for (int i = 0; i < NeopixelNum; i++) pixels[i].begin();

    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_BROWNOUT) {
        // 브라운아웃: 보라색 10번 빠르게 점멸
        for (int i = 0; i < 10; i++) {
            AllNeoOn(PURPLE);
            delay(100);
            AllNeoOn(BLACK);
            delay(100);
        }
    } else if (reason == ESP_RST_PANIC) {
        // 크래시/익셉션: 빨간색 10번 빠르게 점멸
        for (int i = 0; i < 10; i++) {
            AllNeoOn(RED);
            delay(100);
            AllNeoOn(BLACK);
            delay(100);
        }
    }

    has2wifi.Setup("badland_ruins", "Code3824@");
    NeopixelInit();
    RfidInit();
    MotorInit();
    EncoderInit();
    NextionInit();
    TimerInit();
    // has2wifi.Setup();
    // has2wifi.Setup("KT_GiGA_6C64","ed46zx1198");
    DataChanged();
}
void loop() 
{
    ptrCurrentMode();
    WifiTimer.run();
    GameTimer.run();
    BlinkTimer.run();
}
