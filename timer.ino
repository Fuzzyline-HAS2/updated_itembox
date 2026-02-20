void TimerInit() {
  wifiTimerId = WifiTimer.setInterval(wifiTime, WifiIntervalFunc);
  gameTimerId = GameTimer.setInterval(gameTime, GameTimerFunc);
  blinkTimerId = BlinkTimer.setInterval(blinkTime, BlinkTimerFunc);
  GameTimer.deleteTimer(gameTimerId);
  BlinkTimer.deleteTimer(blinkTimerId);
}

void WifiIntervalFunc() {
  // has2wifi.Loop()는 loop()에서 직접 호출 (WDT 방지)
}
void GameTimerFunc() {
  nGameTimerCnt++;
  Serial.println("GameTimerCnt:" + String(nGameTimerCnt));
  if (nGameTimerCnt >= 6) {
    ActivateFunc();
    ledcWrite(VIBRATION_RANGE_PIN, 0);
    GameTimer.deleteTimer(gameTimerId); // 게임 타이머 시작
    WifiTimer.deleteTimer(wifiTimerId);
    wifiTimerId = WifiTimer.setInterval(wifiTime, WifiIntervalFunc);
    nGameTimerCnt = 0;
    detachInterrupt(encoderPinA); // 엔코더 사용 막기
    detachInterrupt(encoderPinB);
    encoderValue = 165;
  }
}

void BlinkTimerFunc() {
  Serial.println("Blink!");
  if (blinkOn == true) {
    lightColor(pixels[blinkNeo], color[blinkColor]);
    blinkOn = false;
  } else {
    lightColor(pixels[blinkNeo], color[BLACK]);
    blinkOn = true;
  }
}
void BlinkTimerStart(int Neo, int NeoColor) {
  blinkNeo = Neo;
  blinkColor = NeoColor;
  blinkTimerId = BlinkTimer.setInterval(blinkTime, BlinkTimerFunc);
}