void DataChanged() {
  static StaticJsonDocument<1000>
      cur; // 저장되어 있는 cur과 읽어온 my 값과 비교후 실행
  if ((String)(const char *)my["game_state"] !=
      (String)(const char *)cur["game_state"]) {
    if ((String)(const char *)my["game_state"] == "setting") {
      SettingFunc();
    } else if ((String)(const char *)my["game_state"] == "ready") {
      ReadyFunc();
    } else if ((String)(const char *)my["game_state"] == "activate") {
      ActivateFunc();
    }
  }
  if ((String)(const char *)my["device_state"] !=
      (String)(const char *)cur["device_state"]) {
    if ((String)(const char *)my["device_state"] == "activate") {
      ActivateFunc();
    } else if ((String)(const char *)my["device_state"] == "used") {
      if (itemBoxUsed == false) {
        transitionTo(ItemBoxState::USED);
        BoxOpen();
        AllNeoOn(RED);
        sendCommand("page pgItemTaken");
        ptrCurrentMode = WaitFunc;
        ptrRfidMode = WaitFunc;
        itemBoxUsed = true;
        BlinkTimer.deleteTimer(blinkTimerId);
        GameTimer.deleteTimer(gameTimerId);
      }
    } else if ((String)(const char *)my["device_state"] ==
               "open") { // 노브 puzzle 끝난상태
      if (itemBoxSelfOpen ==
          false) { // 플레이어가 스스로 열었으면 true 여서 이미 열었다는 의미
        transitionTo(ItemBoxState::OPEN);
        Serial.println("PuzzleSolved");
        AllNeoOn(BLUE);
        sendCommand("page pgItemOpen");
        delay(10);
        sendCommand("wOutTagged.en=1");
        ExpSend();
        BatteryPackSend();
        BoxOpen();
        lightColor(pixels[INNER], color[YELLOW]);
        ptrCurrentMode = RfidLoopInner;
        ptrRfidMode = ItemTook;
        BlinkTimer.deleteTimer(blinkTimerId);
        BlinkTimerStart(INNER, YELLOW); // 내부태그 노란색 점멸 시작
        GameTimer.deleteTimer(
            gameTimerId); // 엔코더 다 푼 이후에는 로그아웃 없이 현 상태 유지
        itemBoxSelfOpen = true;
      }
    } else if ((String)(const char *)my["device_state"] == "close") {
      transitionTo(ItemBoxState::CLOSE);
      BoxClose();
    } else if ((String)(const char *)my["device_state"] == "repaired_all") {
      transitionTo(ItemBoxState::REPAIRED_ALL);
      ptrCurrentMode = WaitFunc;
      ptrRfidMode = WaitFunc;
      AllNeoOn(BLUE);
      BoxOpen();
      sendCommand("page pgEscapeOpen");
    } else if ((String)(const char *)my["device_state"] == "player_win") {
      transitionTo(ItemBoxState::PLAYER_WIN);
      ptrCurrentMode = WaitFunc;
      ptrRfidMode = WaitFunc;
      AllNeoOn(BLUE);
      BoxOpen();
      sendCommand("page pgPlayerWin");
    } else if ((String)(const char *)my["device_state"] == "player_lose") {
      transitionTo(ItemBoxState::PLAYER_LOSE);
      ptrCurrentMode = WaitFunc;
      ptrRfidMode = WaitFunc;
      AllNeoOn(RED);
      BoxOpen();
      sendCommand("page pgPlayerLose");
    }
  }
  cur = my; // cur 데이터 그룹에 현재 읽어온 데이터 저장
}
void WaitFunc(void) {}
void SettingFunc(void) {
  transitionTo(ItemBoxState::SETTING);
  sendCommand("page pgWait");
  Serial.println("SETTING");
  AllNeoOn(WHITE);
  BoxOpen();
  encoderValue = 165;
  answerCnt = 0;
  ptrCurrentMode = WaitFunc;
  ptrRfidMode = WaitFunc;
  itemBoxSelfOpen = false; // 퍼즐함수 성공했는지 확인하는 변수초기화
  itemBoxUsed = false; // 박스 사용했는지 확인하는 변수 초기화
  BlinkTimer.deleteTimer(blinkTimerId);
  GameTimer.deleteTimer(gameTimerId);
  ledcWrite(VIBRATION_RANGE_PIN, 0);
}
void ActivateFunc(void) {
  transitionTo(ItemBoxState::ACTIVATE);
  sendCommand("page pgWait");
  encoderValue = 165;
  answerCnt = 0;
  Serial.println("ACTIVATE");
  AllNeoOn(YELLOW);
  BoxClose();
  ptrCurrentMode = RfidLoopOutter;
  ptrRfidMode = StartPuzzle;
  itemBoxSelfOpen = false; // 퍼즐함수 성공했는지 확인하는 변수초기화
  itemBoxUsed = false; // 박스 사용했는지 확인하는 변수 초기화
  BlinkTimer.deleteTimer(blinkTimerId);
  GameTimer.deleteTimer(gameTimerId);
  ledcWrite(VIBRATION_RANGE_PIN, 0);
}
void ReadyFunc(void) {
  transitionTo(ItemBoxState::READY);
  int nCycleCnt = 0;
  for (int i = HI1; i <= FI2; i++) {
    batteryPackRnd[i].nVal = 0;
    Serial.println(batteryPackRnd[i].strDevice + ":" +
                   (String)batteryPackRnd[i].nVal);
  }
  while (nCycleCnt < 10) {
    long rndDevice = random(0, 10);
    if (batteryPackRnd[rndDevice].nVal < 3) {
      batteryPackRnd[rndDevice].nVal++;
      nCycleCnt++;
    }
  }
  for (int i = HI1; i <= FI2; i++)
    Serial.println(batteryPackRnd[i].strDevice + ":" +
                   (String)batteryPackRnd[i].nVal);
  for (int i = HI1; i <= FI2; i++) {
    if (batteryPackRnd[i].nVal != 0)
      has2wifi.Send(batteryPackRnd[i].strDevice, "battery_pack",
                    (String)batteryPackRnd[i].nVal);
  }
  sendCommand("page pgWait");
  Serial.println("READY");
  AllNeoOn(RED);
  BoxClose();
  ptrCurrentMode = WaitFunc;
  ptrRfidMode = WaitFunc;
  itemBoxSelfOpen = false; // 퍼즐함수 성공했는지 확인하는 변수초기화
  itemBoxUsed = false; // 박스 사용했는지 확인하는 변수 초기화
  BlinkTimer.deleteTimer(blinkTimerId);
  GameTimer.deleteTimer(gameTimerId);
  ledcWrite(VIBRATION_RANGE_PIN, 0);
}