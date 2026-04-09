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
    if (boxMotorRunning) {
        if (boxClosing && digitalRead(BOXSWITCH_PIN) == HIGH) {  // HIGH = 닫힘 감지
            MotorStop();
            boxMotorRunning = false;
            Serial.println("BOX Closed");
        } else if (!boxClosing && digitalRead(BOXSWITCH_PIN) == LOW) {  // LOW = 열림 감지
            MotorStop();
            boxMotorRunning = false;
            Serial.println("BOX Opened");
            if (pendingOpenScreen) {
                sendCommand("page pgItemOpen");
                pendingOpenScreen = false;
            }
        }
    }
    ptrCurrentMode();
    WifiTimer.run();
    GameTimer.run();
    BlinkTimer.run();
}
