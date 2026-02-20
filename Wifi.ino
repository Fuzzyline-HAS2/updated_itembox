static String pendingGameState = "";
static String pendingDeviceState = "";
static bool pendingGameStateChanged = false;
static bool pendingDeviceStateChanged = false;

void DataChanged() {
  static StaticJsonDocument<1000> cur;
  String myGameState = (String)(const char *)my["game_state"];
  String curGameState = (String)(const char *)cur["game_state"];
  if (myGameState != curGameState) {
    pendingGameState = myGameState;
    pendingGameStateChanged = true;
  }

  String myDeviceState = (String)(const char *)my["device_state"];
  String curDeviceState = (String)(const char *)cur["device_state"];
  if (myDeviceState != curDeviceState) {
    pendingDeviceState = myDeviceState;
    pendingDeviceStateChanged = true;
  }

  cur = my;
}

void ProcessServerStateChanges() {
  if (pendingGameStateChanged) {
    pendingGameStateChanged = false;
    if (pendingGameState == "setting") {
      SettingFunc();
    } else if (pendingGameState == "ready") {
      ReadyFunc();
    } else if (pendingGameState == "activate") {
      ActivateFunc();
    }
  }

  if (!pendingDeviceStateChanged) {
    return;
  }

  pendingDeviceStateChanged = false;
  if (pendingDeviceState == "activate") {
    ActivateFunc();
  } else if (pendingDeviceState == "used") {
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
  } else if (pendingDeviceState == "open") {
    if (itemBoxSelfOpen == false) {
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
      BlinkTimerStart(INNER, YELLOW);
      GameTimer.deleteTimer(gameTimerId);
      itemBoxSelfOpen = true;
    }
  } else if (pendingDeviceState == "close") {
    transitionTo(ItemBoxState::CLOSE);
    BoxClose();
  } else if (pendingDeviceState == "repaired_all") {
    transitionTo(ItemBoxState::REPAIRED_ALL);
    ptrCurrentMode = WaitFunc;
    ptrRfidMode = WaitFunc;
    AllNeoOn(BLUE);
    BoxOpen();
    sendCommand("page pgEscapeOpen");
  } else if (pendingDeviceState == "player_win") {
    transitionTo(ItemBoxState::PLAYER_WIN);
    ptrCurrentMode = WaitFunc;
    ptrRfidMode = WaitFunc;
    AllNeoOn(BLUE);
    BoxOpen();
    sendCommand("page pgPlayerWin");
  } else if (pendingDeviceState == "player_lose") {
    transitionTo(ItemBoxState::PLAYER_LOSE);
    ptrCurrentMode = WaitFunc;
    ptrRfidMode = WaitFunc;
    AllNeoOn(RED);
    BoxOpen();
    sendCommand("page pgPlayerLose");
  }
}

void enqueueReadyBatterySends() {
  readySendPending = true;
  readySendIndex = HI1;
  readySendLastMs = 0;
}

void ReadySendService() {
  if (!enableReadyPackSync) {
    readySendPending = false;
    return;
  }

  if (!readySendPending) return;

  unsigned long now = millis();
  if (readySendLastMs != 0 && (now - readySendLastMs) < readySendIntervalMs) {
    return;
  }

  while (readySendIndex <= FI2) {
    if (batteryPackRnd[readySendIndex].nVal != 0) {
      has2wifi.Send(batteryPackRnd[readySendIndex].strDevice, "battery_pack",
                    (String)batteryPackRnd[readySendIndex].nVal);
      readySendLastMs = now;
      readySendIndex++;
      yield();
      return;
    }
    readySendIndex++;
  }

  readySendPending = false;
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
  itemBoxSelfOpen = false;
  itemBoxUsed = false;
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
  itemBoxSelfOpen = false;
  itemBoxUsed = false;
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
  if (enableReadyPackSync) {
    enqueueReadyBatterySends();
  }
  sendCommand("page pgWait");
  Serial.println("READY");
  AllNeoOn(RED);
  BoxClose();
  ptrCurrentMode = WaitFunc;
  ptrRfidMode = WaitFunc;
  itemBoxSelfOpen = false;
  itemBoxUsed = false;
  BlinkTimer.deleteTimer(blinkTimerId);
  GameTimer.deleteTimer(gameTimerId);
  ledcWrite(VIBRATION_RANGE_PIN, 0);
}
