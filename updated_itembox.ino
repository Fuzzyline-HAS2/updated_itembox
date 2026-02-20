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

  // QC System Initialization
  QCEngine::getInstance().begin(1000);
  QCEngine::getInstance().addRule(new QCRule_WifiConnection());
  QCEngine::getInstance().addRule(new QCRule_WifiSignal());
  QCEngine::getInstance().addRule(new QCRule_HeapMemory());
  QCEngine::getInstance().addRule(new QCRule_ResetReason());
  QCEngine::getInstance().addRule(new QCRule_PinConflict());
  QCEngine::getInstance().addRule(new QCRule_GpioCapability());
  QCEngine::getInstance().addRule(new QCRule_GameState());
  QCEngine::getInstance().addRule(new QCRule_RfidStatus());
  QCEngine::getInstance().addRule(new QCRule_RfidHeartbeat());
  QCEngine::getInstance().addRule(new QCRule_EncoderRange());
  QCEngine::getInstance().addRule(new QCRule_StateConsistency());
  Serial.println("QC System Started");
}
void loop() {
  ptrCurrentMode();
  WifiTimer.run();
  GameTimer.run();
  BlinkTimer.run();

  // QC Check
  QCEngine::getInstance().tick();
}
