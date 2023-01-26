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

#include "itembox_random_add.h"

void setup() 
{
    Serial.begin(115200);
    NeopixelInit();
    RfidInit();
    MotorInit();
    EncoderInit();
    NextionInit();
    TimerInit();
    has2wifi.Setup();
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
