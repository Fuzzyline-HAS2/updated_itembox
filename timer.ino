void TimerInit()
{
    wifiTimerId = WifiTimer.setInterval(wifiTime,WifiIntervalFunc);
    gameTimerId = GameTimer.setInterval(gameTime,GameTimerFunc);
    blinkTimerId = BlinkTimer.setInterval(blinkTime,BlinkTimerFunc);
    GameTimer.deleteTimer(gameTimerId);
    BlinkTimer.deleteTimer(blinkTimerId); 
}

void WifiIntervalFunc()
{
    has2wifi.Loop(DataChanged);
}
void GameTimerFunc(){
    nGameTimerCnt++;
    Serial.println("GameTimerCnt:"+String(nGameTimerCnt));
    if(nGameTimerCnt >= 6){
        ActivateFunc();
        ledcWrite(VibrationLedChannel, 0);
        GameTimer.deleteTimer(gameTimerId);        //게임 타이머 시작
        WifiTimer.deleteTimer(wifiTimerId);
        wifiTimerId = WifiTimer.setInterval(wifiTime,WifiIntervalFunc);
        nGameTimerCnt = 0;
        detachInterrupt(encoderPinA);           //엔코더 사용 막기
        detachInterrupt(encoderPinB);
        encoderValue = 165;
    }
}

void BlinkTimerFunc(){
    Serial.println("Blink!");
    if(blinkOn == true){
        pixels[blinkNeo].lightColor(color[blinkColor]);
        blinkOn = false;
    }
    else{
        pixels[blinkNeo].lightColor(color[BLACK]);
        blinkOn = true;
    }

}
void BlinkTimerStart(int Neo, int NeoColor){
    blinkNeo = Neo;
    blinkColor = NeoColor;
    blinkTimerId = BlinkTimer.setInterval(blinkTime,BlinkTimerFunc);
}