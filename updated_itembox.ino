; /**
   * @file Done_ItemBox_code.ino
   * @author 김병준 (you@domain.com)
   * @brief
   * @version 1.0
   * @date 2022-11-29
   *
   * @copyright Copyright (c) 2022
   *
   */

#include "QC/QC_Engine.h"
#include "QC/QC_Rules.h"
#include "updated_itembox.h"


void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.printf("[BOOT] reset_reason=%d\n", (int)esp_reset_reason());
  Serial.println("[BOOT] has2wifi.Setup start");
  has2wifi.Setup("badland_ruins", "Code3824@");
  Serial.println("[BOOT] has2wifi.Setup done");
  Serial.println("[BOOT] NeopixelInit start");
  NeopixelInit();
  Serial.println("[BOOT] NeopixelInit done");
  Serial.println("[BOOT] RfidInit start");
  RfidInit();
  Serial.println("[BOOT] RfidInit done");
  Serial.println("[BOOT] MotorInit start");
  MotorInit();
  Serial.println("[BOOT] MotorInit done");
  Serial.println("[BOOT] EncoderInit start");
  EncoderInit();
  Serial.println("[BOOT] EncoderInit done");
  Serial.println("[BOOT] NextionInit start");
  NextionInit();
  Serial.println("[BOOT] NextionInit done");
  Serial.println("[BOOT] TimerInit start");
  TimerInit();
  Serial.println("[BOOT] TimerInit done");
  // has2wifi.Setup("KT_GiGA_6C64","ed46zx1198");
  Serial.println("[BOOT] DataChanged start");
  DataChanged();
  Serial.println("[BOOT] DataChanged done");

  // QC System Initialization
  QCEngine::getInstance().begin(1000);
  QCEngine::getInstance().addRule(new QCRule_WifiConnection());
  QCEngine::getInstance().addRule(new QCRule_WifiSignal());
  QCEngine::getInstance().addRule(new QCRule_HeapMemory());
  QCEngine::getInstance().addRule(new QCRule_GameState());
  QCEngine::getInstance().addRule(new QCRule_RfidStatus());
  QCEngine::getInstance().addRule(new QCRule_EncoderRange());
  QCEngine::getInstance().addRule(new QCRule_StateConsistency());
  Serial.println("QC System Started");
}
void loop() {
  ProcessServerStateChanges();
  ptrCurrentMode();
  WifiTimer.run();
  GameTimer.run();
  BlinkTimer.run();
  MotorService();
  ReadySendService();

  // QC Check
  QCEngine::getInstance().tick();
}
