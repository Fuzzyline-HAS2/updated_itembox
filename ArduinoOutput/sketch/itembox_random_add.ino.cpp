#include <Arduino.h>
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\itembox_random_add.ino"
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

#line 14 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\itembox_random_add.ino"
void setup();
#line 27 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\itembox_random_add.ino"
void loop();
#line 4 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\Game_system.ino"
void Puzzle(void);
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\Wifi.ino"
void DataChanged();
#line 69 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\Wifi.ino"
void WaitFunc(void);
#line 73 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\Wifi.ino"
void SettingFunc(void);
#line 89 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\Wifi.ino"
void ActivateFunc(void);
#line 105 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\Wifi.ino"
void ReadyFunc(void);
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\encoder.ino"
void EncoderInit();
#line 9 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\encoder.ino"
void updateEncoder();
#line 26 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\encoder.ino"
long readEncoderValue(void);
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\motor.ino"
void MotorInit();
#line 19 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\motor.ino"
void BoxClose();
#line 29 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\motor.ino"
void BoxOpen();
#line 39 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\motor.ino"
void MotorStop();
#line 46 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\motor.ino"
void EncoderVibrationStrength(int answer);
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\neopixel.ino"
void NeopixelInit();
#line 16 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\neopixel.ino"
void EncoderNeopixelOn();
#line 25 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\neopixel.ino"
void NeoBlink(int neo, int neoColor, int cnt, int blinkTime);
#line 35 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\neopixel.ino"
void AllNeoOn(int neoColor);
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\nextion.ino"
void NextionInit();
#line 7 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\nextion.ino"
void DisplayCheck();
#line 16 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\nextion.ino"
void NextionReceived(String *nextion_string);
#line 25 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\nextion.ino"
void ExpSend();
#line 42 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\nextion.ino"
void BatteryPackSend();
#line 4 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\rfid.ino"
void RfidInit();
#line 30 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\rfid.ino"
void RfidLoopInner();
#line 58 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\rfid.ino"
void RfidLoopOutter();
#line 87 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\rfid.ino"
void CheckingPlayers(uint8_t rfidData[32]);
#line 115 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\rfid.ino"
void StartPuzzle();
#line 131 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\rfid.ino"
void PuzzleSolved();
#line 153 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\rfid.ino"
void ItemTook();
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\timer.ino"
void TimerInit();
#line 10 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\timer.ino"
void WifiIntervalFunc();
#line 14 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\timer.ino"
void GameTimerFunc();
#line 30 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\timer.ino"
void BlinkTimerFunc();
#line 42 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\timer.ino"
void BlinkTimerStart(int Neo, int NeoColor);
#line 14 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\itembox_random_add.ino"
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

#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\Game_system.ino"
/**
 * @brief 현재 풀고있는 문제에서 엔코더 값의 변화에 따라 네오픽셀 진동모터 정답카운팅을 관장하는 함수
 */
void Puzzle(void)
{
    int currentAnswer = modeValue[ANSWER][answerCnt];   // Puzzle 함수를 진행하는 동안 현재의 정답 저장용 변수, 몇번째 문제인지 저장하는건 answerCnt 전연 변수
    EncoderNeopixelOn();                                // 현재 엔코더 위치 적색으로 표현하기 위해 네오픽셀 켜주는 함수 
    EncoderVibrationStrength(currentAnswer);            // 현재 엔코더 위치에 따라 진동모터 세기 결정해주는 함수

    if (digitalRead(buttonPin) == LOW)                                                                      // 엔코더 스위치 눌렸을때
    {
        volatile long currentEncoderValue = encoderValue;                                                   // EnocoderRead 함수에서 엔코더값을 저장한 전역변수 encoderValue 복사
        long differenceValue = (abs(currentAnswer - (encoderValue / 4))) / modeValue[RANGE][ANSWER_RANGE];  // 정답 범위에서 현재 엔코더 갑이 얼마나 차이나는지 확인하는 변수
        if (differenceValue == 0)               // 정답일때
        {
            Serial.println("Correct Answer");
            NeoBlink(ENCODER, GREEN, 5, 250);   // 엔코더 네오픽셀 적색 0.25s 간격으로 5번 점멸 -> Delay사용으로 이 함수에 2초 머물러 있음
            answerCnt++;                        // 정답시 다음 문제로 넘어가기 위해 카운트 +1
            if (answerCnt >= modeValue[RANGE][ANSWER_CNT])                        // 모든 정답을 맞추었을때
            {
                Serial.println("QUIZ SUCCEED");                                                 
                sendCommand("wQuizSolved.en=1");                                    // Nextion으로 "해제 완료" 나레이션 출력 명령 전송
                ledcWrite(VibrationLedChannel, 0);                                  // 진동모터 끄기
                answerCnt = 0;                                                      // Puzzle의 현재정답 저장변수 초기화
                detachInterrupt(encoderPinA);                                       // 하드웨어 인터럽트 종료 -> 엔코더 사용 막기 위해
                detachInterrupt(encoderPinB);
                BlinkTimer.deleteTimer(blinkTimerId);                               // 황색점멸 타이머 초기화 위해 종료
                BlinkTimerStart(PN532, YELLOW);                                     // 외부 RFID 네오 황색 점멸
                WifiTimer.deleteTimer(wifiTimerId);
                wifiTimerId = WifiTimer.setInterval(wifiTime, WifiIntervalFunc);    // 엔코더 종료되어서 와이파이 다시 활성화
                nGameTimerCnt = 0;                                                  // 게임 타이머 초기화
                ptrCurrentMode = RfidLoopOutter;                                    // ptr 메인 함수 Puzzle -> RFIDOutter로 변경: 태그하여 노브 끝내기 위해
                ptrRfidMode = PuzzleSolved;                                         // ChekingPlayer 실행시 실행되는 ptr함수 주소가 WaitFunc -> puzzleSolved로 변경: 아박 열기위해
            }
        }
        else                                    // 틀렸을때
        {
            Serial.println("Wrong Answer");     
            NeoBlink(ENCODER, RED, 5, 250);     //엔코더 네오픽셀 적색 0.25s 간격으로 5번 점멸 -> Delay사용으로 이 함수에 2초 머물러 있음
        }
        encoderValue = currentEncoderValue;     // 네오픽셀 점멸 시 마지막으로 저장된 엔코더 값 저장해서 현재 엔코더 값이 바뀌어도 되돌아가게 하는 변수 저장
    }
}

#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\Wifi.ino"
void DataChanged()
{
  static StaticJsonDocument<500> cur;   //저장되어 있는 cur과 읽어온 my 값과 비교후 실행
    if((String)(const char*)my["game_state"] != (String)(const char*)cur["game_state"])
    {  
        if((String)(const char*)my["game_state"] == "setting"){
            SettingFunc();
        }
        else if((String)(const char*)my["game_state"] == "ready"){
            ReadyFunc();
        }
        else if((String)(const char*)my["game_state"] == "activate"){
            ActivateFunc();
        }
    }
    if((String)(const char*)my["device_state"] != (String)(const char*)cur["device_state"])
    {
        if((String)(const char*)my["device_state"] == "activate"){
            ActivateFunc();
        }
        else if((String)(const char*)my["device_state"] == "used"){
            if(itemBoxUsed == false){
                BoxOpen();
                AllNeoOn(RED);
                sendCommand("page pgItemTaken");
                ptrCurrentMode = WaitFunc;
                ptrRfidMode = WaitFunc; 
                itemBoxUsed = true;
                BlinkTimer.deleteTimer(blinkTimerId);
                GameTimer.deleteTimer(gameTimerId);
            }
        }
        else if((String)(const char*)my["device_state"] == "open"){ //노브 puzzle 끝난상태
            if(itemBoxSelfOpen == false){                           //플레이어가 스스로 열었으면 true 여서 이미 열었다는 의미
                Serial.println("PuzzleSolved");
                AllNeoOn(BLUE);
                sendCommand("page pgItemOpen");
                delay(10);
                sendCommand("wOutTagged.en=1");
                ExpSend();
                BatteryPackSend();
                BoxOpen();
                pixels[INNER].lightColor(color[YELLOW]);
                ptrCurrentMode = RfidLoopInner;
                ptrRfidMode = ItemTook;
                BlinkTimer.deleteTimer(blinkTimerId);
                BlinkTimerStart(INNER, YELLOW);                     //내부태그 노란색 점멸 시작
                GameTimer.deleteTimer(gameTimerId);                 // 엔코더 다 푼 이후에는 로그아웃 없이 현 상태 유지
                itemBoxSelfOpen = true;
            }
        }
        else if((String)(const char*)my["device_state"] == "close"){ 
            BoxClose();
        }
        else if((String)(const char*)my["device_state"] == "player_win"){ 
            AllNeoOn(BLUE);
            BoxOpen();
            sendCommand("page pgPlayerWin");
        }
        else if((String)(const char*)my["device_state"] == "player_lose"){ 
            AllNeoOn(RED);
            BoxOpen();
            sendCommand("page pgPlayerLose");
        }
        
    }
  cur = my; // cur 데이터 그룹에 현재 읽어온 데이터 저장
}
void WaitFunc(void)
{

}
void SettingFunc(void)
{
    sendCommand("page pgWait");
    Serial.println("SETTING");
    AllNeoOn(WHITE);
    BoxOpen();
    encoderValue = 165;
    answerCnt = 0;
    ptrCurrentMode = WaitFunc;
    ptrRfidMode = WaitFunc;
    itemBoxSelfOpen = false;    //퍼즐함수 성공했는지 확인하는 변수초기화
    itemBoxUsed = false;        //박스 사용했는지 확인하는 변수 초기화
    BlinkTimer.deleteTimer(blinkTimerId);
    GameTimer.deleteTimer(gameTimerId);
    ledcWrite(VibrationLedChannel, 0);
}
void ActivateFunc(void)
{
    sendCommand("page pgWait");
    encoderValue = 165;
    answerCnt = 0;
    Serial.println("ACTIVATE");
    AllNeoOn(YELLOW);
    BoxClose();
    ptrCurrentMode = RfidLoopOutter;
    ptrRfidMode = StartPuzzle;
    itemBoxSelfOpen = false;    //퍼즐함수 성공했는지 확인하는 변수초기화
    itemBoxUsed = false;        //박스 사용했는지 확인하는 변수 초기화
    BlinkTimer.deleteTimer(blinkTimerId);
    GameTimer.deleteTimer(gameTimerId);
    ledcWrite(VibrationLedChannel, 0);
}
void ReadyFunc(void)
{
    int nCycleCnt = 0;
    for(int i = HI1; i <= FI2; i++){
        batteryPackRnd[i].nVal = 0;
        Serial.println(batteryPackRnd[i].strDevice + ":" + (String)batteryPackRnd[i].nVal);
    }
    while(nCycleCnt < 10){
        long rndDevice = random(0,10);
        if(batteryPackRnd[rndDevice].nVal < 3)
        {
            batteryPackRnd[rndDevice].nVal++;
            nCycleCnt++;
        }
    }
    for(int i = HI1; i <= FI2; i++)
        Serial.println(batteryPackRnd[i].strDevice + ":" + (String)batteryPackRnd[i].nVal);
    for(int i = HI1; i <= FI2; i++){
        if(batteryPackRnd[i].nVal != 0)
            has2wifi.Send(batteryPackRnd[i].strDevice, "battery_pack", (String)batteryPackRnd[i].nVal);
    }
    sendCommand("page pgWait");
    Serial.println("READY");
    AllNeoOn(RED);
    BoxClose();
    ptrCurrentMode = WaitFunc;
    ptrRfidMode = WaitFunc;
    itemBoxSelfOpen = false;    //퍼즐함수 성공했는지 확인하는 변수초기화
    itemBoxUsed = false;        //박스 사용했는지 확인하는 변수 초기화
    BlinkTimer.deleteTimer(blinkTimerId);
    GameTimer.deleteTimer(gameTimerId);
    ledcWrite(VibrationLedChannel, 0);
}
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\encoder.ino"
void EncoderInit()
{
    Serial.println("ENCODER INIT");     //엔코더 핀 관련 초기화
    pinMode(encoderPinA, INPUT_PULLUP);
    pinMode(encoderPinB, INPUT_PULLUP);
    pinMode(buttonPin, INPUT_PULLUP);
}

void updateEncoder()
{
    nGameTimerCnt = 0;                      //게임 타이머 초기화
    int MSB = digitalRead(encoderPinA);     // MSB = most significant bit
    int LSB = digitalRead(encoderPinB);     // LSB = least significant bit
    
    int encoded = (MSB << 1) | LSB;         // converting the 2 pin value to single number
    int sum = (lastEncoded << 2) | encoded; // adding it to the previous encoded value
    
    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++; 
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;
    lastEncoded = encoded;                  // store this value for next time
       
    if(encoderValue > 380)      encoderValue = 380;  // (0 ~ 95)*4엔코더 값 최대최소 제한 걸어두기                
    else if(encoderValue < 0)   encoderValue = 0;
}
   
long readEncoderValue(void)     //엔코더읽은 값 / 4 해주는 함수: 한틱 돌아갈떄 4만큼 증가해서 해둠: 하드웨어 인터럽트가 change이어서 4틱 바뀜
{
    return encoderValue / 4;
}

#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\motor.ino"
void MotorInit()
{
    //Linear Motor Init
    pinMode(BOXSWITCH_PIN,INPUT_PULLUP);
    pinMode(MOTOR_INA1_PIN, OUTPUT);
    pinMode(MOTOR_INA2_PIN, OUTPUT);
    ledcSetup(MotorLedChannel, MotorFreq, MotorResolution);
    ledcAttachPin(MOTOR_PWMA_PIN, MotorLedChannel);
    ledcWrite(MotorLedChannel, 0);
    BoxClose();
    
    //Vibration Motor Init
    pinMode(VIBRATION_ANSWER_PIN, OUTPUT);
    digitalWrite(VIBRATION_ANSWER_PIN, HIGH);
    ledcSetup(VibrationLedChannel, MotorFreq, MotorResolution);
    ledcAttachPin(VIBRATION_RANGE_PIN, VibrationLedChannel);
    ledcWrite(VibrationLedChannel, 0);
}
void BoxClose()
{
    Serial.println("BOX Close");
    ledcWrite(MotorLedChannel, MotorMAX_DUTY_CYCLE - 1);
    digitalWrite(MOTOR_INA1_PIN, LOW);
    digitalWrite(MOTOR_INA2_PIN, HIGH);
    delay(4000);
    Serial.println("BOX Closed");
}

void BoxOpen()
{
    Serial.println("BOX Open");
    ledcWrite(MotorLedChannel, MotorMAX_DUTY_CYCLE - 1);
    digitalWrite(MOTOR_INA1_PIN, HIGH);
    digitalWrite(MOTOR_INA2_PIN, LOW);
    delay(4000);
    Serial.println("BOX Opened");
}

void MotorStop()
{
    Serial.println("모터 스탑");
    digitalWrite(MOTOR_INA1_PIN, LOW);
    digitalWrite(MOTOR_INA2_PIN, LOW);
}

void EncoderVibrationStrength(int answer)
{
    int differenceValue = abs(answer - (encoderValue/4));
    int answerRange = modeValue[RANGE][1];
    int vibeRange = modeValue[RANGE][2];
    int vibeStrength = 0;
    if(differenceValue < answerRange + vibeRange * 0)       vibeStrength = 0;
    else if(differenceValue < answerRange + vibeRange * 1)  vibeStrength = 1;
    else if(differenceValue < answerRange + vibeRange * 2)  vibeStrength = 2;
    else if(differenceValue < answerRange + vibeRange * 3)  vibeStrength = 3;
    else vibeStrength = 4;
    ledcWrite(VibrationLedChannel, modeValue[VIBESTREGNTH][vibeStrength]);
}

#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\neopixel.ino"
void NeopixelInit() // 전체 네오픽셀 초기화 해주는 함수                    
{
  for (int i = 0; i < NeopixelNum; ++i)
  {
    pixels[i].begin();
  }
  for (int i = 0; i < NeopixelNum; ++i)
  {
    pixels[i].lightColor(color[WHITE]);
  }
}

/**
 * @brief 현재 엔코더 값을 네오픽셀의 빨간색으로 표시하는 함수
 */
void EncoderNeopixelOn()
{
    int neoColor = readEncoderValue() / 24;       // 0~23, 24~4 ... 24마다 네오픽셀의 밝기가 증가함을 저장하는 변수
    int neoNum = 23 - (readEncoderValue() % 24);  // 현재 빨간색으로 표시되어야 할 네오픽셀 번호를 저장하는 변수
    for(int i = 0; i < NumPixels[ENCODER]; i++)
        pixels[ENCODER].setPixelColor(i,pixels[ENCODER].Color(color[neoColor+7][0], color[neoColor+7][1], color[neoColor+7][2])); // ENCODERGREEN0부터 시작하기 위해 + 7 더해줌
     pixels[ENCODER].setPixelColor(neoNum,pixels[ENCODER].Color(color[RED][0],color[RED][1],color[RED][2])); 
     pixels[ENCODER].show(); 
}
void NeoBlink(int neo, int neoColor, int cnt, int blinkTime)
{
  for(int i = 0; i < cnt; i++){                          //0.5*10=5초동안 점멸
    pixels[neo].lightColor(color[BLACK]); //전체 off
    delay(blinkTime);            
    pixels[neo].lightColor(color[neoColor]); //전체 적색on
    delay(blinkTime);                   //전체 적색on
  }
}

void AllNeoOn(int neoColor){
  for (int i = 0; i < NeopixelNum; ++i)
    pixels[i].lightColor(color[neoColor]);
}

#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\nextion.ino"
void NextionInit()
{
   nexInit();
   nexHwSerial.begin(9600, SERIAL_8N1, NEXTIONHWSERIAL_RX_PIN, NEXTIONHWSERIAL_TX_PIN);
}

void DisplayCheck()
{
 while (nexHwSerial.available() > 0)
 {
   String nextion_string = nexHwSerial.readStringUntil(' ');
   NextionReceived(&nextion_string);
 }
}

void NextionReceived(String *nextion_string)
{
 if (*nextion_string == "test")
 {
   sendCommand("page pgItemTaken");
   sendCommand("wQuizSolved.en=1");
 }
}

void ExpSend(){
    if((String)(const char*)my["exp_pack"] == "10")
    {
        sendCommand("picExp.pic=0");
        sendCommand("pgItemOpen.vExp.val=10");
    }
    else if((String)(const char*)my["exp_pack"] == "30")
    {
        sendCommand("picExp.pic=1");
        sendCommand("pgItemOpen.vExp.val=30");
    }
    else if((String)(const char*)my["exp_pack"] == "50")
    {
        sendCommand("picExp.pic=2");
        sendCommand("pgItemOpen.vExp.val=50");
    }
}
void BatteryPackSend(){
    if((String)(const char*)my["battery_pack"] == "1")
    {
        sendCommand("picBatteryPack.pic=3");
        sendCommand("pgItemOpen.vBatteryPack.val=1");
    }
    else if((String)(const char*)my["battery_pack"] == "2")
    {
        sendCommand("picBatteryPack.pic=4");
        sendCommand("pgItemOpen.vBatteryPack.val=2");
    }
    else if((String)(const char*)my["battery_pack"] == "3")
    {
        sendCommand("picBatteryPack.pic=5");
        sendCommand("pgItemOpen.vBatteryPack.val=3");
    }
    else if((String)(const char*)my["battery_pack"] == "4")
    {
        sendCommand("picBatteryPack.pic=6");
        sendCommand("pgItemOpen.vBatteryPack.val=4");
    }
    else if((String)(const char*)my["battery_pack"] == "5")
    {
        sendCommand("picBatteryPack.pic=7");
        sendCommand("pgItemOpen.vBatteryPack.val=5");
    }
}


#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\rfid.ino"
/**
 * @brief 내부 외부 pn532 초기활성화 및 실패시  goto문 반복
 */
void RfidInit()
{
RestartPn532:                                             // goto문 회귀 위치
  for (int i = 0; i < rfid_num; ++i)
  {
    nfc[i].begin();
    if (!(nfc[i].getFirmwareVersion()))                   // pn532 동작 안할때
    { 
      Serial.println("PN532 연결실패 : " + String(i));
      AllNeoOn(RED);
      Serial.println("pn532 INIT 재실행");
      goto RestartPn532;                                  // pn532오류시 goto문 동작하여 RFIDinit함수 재실행
    }
    else
    {
      nfc[i].SAMConfig();
      Serial.println("PN532 연결성공 : " + String(i));
      rfid_init_complete[i] = true;
      AllNeoOn(YELLOW);
    }
    delay(100);
  }
}
/**
 * @brief 아이템박스 내부 pn532 태그 읽어와서 CheckingPlayer로 전송
 */
void RfidLoopInner()
{
  uint8_t uid[3][7] = {{0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0, 0, 0}}; // Buffer to store the returned UID
  uint8_t uidLength[] = {0};                   // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t data[32];
  byte pn532_packetbuffer11[64];
  pn532_packetbuffer11[0] = 0x00;
  if (nfc[INPN532].sendCommandCheckAck(pn532_packetbuffer11, 1))
  {                                                                           // rfid 통신 가능한 상태인지 확인
    if (nfc[INPN532].startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A))
    {                                                                         // rfid에 tag 찍혔는지 확인용 //데이터 들어오면 uid정보 가져오기
      if (nfc[INPN532].ntag2xx_ReadPage(7, data))
      {                                                                       // ntag 데이터에 접근해서 불러와서 data행열에 저장
        Serial.println("TAGGGED");
        CheckingPlayers(data);
      }
    }
  }
  // TODO InnerRFID 루프시 연결 안되면 워치독
  // else(
  //   ESP.restart();
  // )
}
/**
 * @brief 아이템박스 외부 pn532 태그 읽어와서 CheckingPlayer로 전송
 */
void RfidLoopOutter()
{
  uint8_t uid[3][7] = {{0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0, 0, 0}}; // Buffer to store the returned UID
  uint8_t uidLength[] = {0};                   // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t data[32];
  byte pn532_packetbuffer11[64];
  pn532_packetbuffer11[0] = 0x00;
  if (nfc[OUTPN532].sendCommandCheckAck(pn532_packetbuffer11, 1))
  {                                                                           // rfid 통신 가능한 상태인지 확인
    if (nfc[OUTPN532].startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A))
    {                                                                         // rfid에 tag 찍혔는지 확인용 //데이터 들어오면 uid정보 가져오기
      if (nfc[OUTPN532].ntag2xx_ReadPage(7, data))
      {                                                                       // ntag 데이터에 접근해서 불러와서 data행열에 저장
        Serial.println("TAGGGED");
        CheckingPlayers(data);
      }
    }
  }
  // TODO OutterRFID 루프시 연결 안되면 워치독
  // else(
  //   ESP.restart();
  // )
}

/**
 * @brief 내외부에서 태그한 카드데이터 string으로 변환후 DB에 요청하여 'role'확인하여 ptrRfidMode로 전송
 */
void CheckingPlayers(uint8_t rfidData[32])                // 어떤 카드가 들어왔는지 확인용
{
  String tagUser = "";                                    // 읽어온 uint_8t값 string으로 변환하기 위한 String 변수
  for (int i = 0; i < 4; i++)                             // GxPx 데이터만 배열에서 추출해서 string으로 저장
    tagUser += (char)rfidData[i];
  Serial.println("tag_user_data : " + tagUser);
  if (tagUser == "MMMM")
  {                                                       //"MMMM"일경우 DB요청 하지 않고 바로 watchdog 실행(DB에 MMMM 플레이어는 존재하지 않아서 요청하면 오류 발생)
    ESP.restart();
  }
                                                          // 1. 태그한 플레이어의 역할과 생명칩갯수, 최대생명칩갯수 등 읽어오기
  has2wifi.Receive(tagUser);                              // 2. 술래인지, 플레이어인지 구분
  if ((String)(const char *)tag["role"] == "player")      // 3. 태그한 사용자가 플레이어 이면
  { 
    Serial.println("Player Tagged");
    ptrRfidMode();
  }
  else if ((String)(const char *)tag["role"] == "tagger") // 4. 태그한 사용자가 술래면 아무 변화 x
    Serial.println("Tagger Tagged");
  else if ((String)(const char *)tag["role"] == "ghost")  // 5. 태그한 사용자가 유령이면 아무 변화 x
    Serial.println("Ghost Tagged");
  else // 6. 예외 처리
    Serial.println("Wrong TAG");
}

/**
 * @brief Activate 상황에서 외부 pn532 태그시 엔코더 활성화 후 RFID 중지 Puzzle함수 실행
 */
void StartPuzzle()
{
  Serial.println("StartPuzzle");
  WifiTimer.deleteTimer(wifiTimerId);                           // 엔코더 렉을 없애기 위해 wifi read 엔코더 사용동안에 중단
  GameTimer.deleteTimer(gameTimerId);
  gameTimerId = GameTimer.setInterval(gameTime, GameTimerFunc); // puzzle 함수 실행동안 n초 이상 입력이 없으면 초기화 하기위해 시간을 재는 타이머 시작
  answerCnt = 0;
  ptrCurrentMode = Puzzle;                                      // ptr함수의 주소를 RFIDOuter -> Puzzle로 변환
  AllNeoOn(BLUE);                                               // puzzle 함수 진행동안 전체 네오픽셀 파란색 유지
  attachInterrupt(encoderPinA, updateEncoder, CHANGE);          // 엔코더 하드웨어 인터럽트 활성화
  attachInterrupt(encoderPinB, updateEncoder, CHANGE);
}

/**
 * @brief Puzzle함수로 문제를 다 맞춘 후 완료하는 태그를 실행했을때 실행되는 함수
 */
void PuzzleSolved()
{
  itemBoxSelfOpen = true;                                                         // 태그하면 아이템박스가 open 상태 임으로 메인에서 open 명령 들어와도 재실행되지 않게 제한하는 bool 변수
  has2wifi.Send((String)(const char *)my["device_name"], "device_state", "open"); // 메인으로 현재부터 아박의 상태가 open으로 저장
  Serial.println("PuzzleSolved");
  AllNeoOn(BLUE);                       // 엔코더 네오픽셀의 빨간색 포인틀 없애기 위해 전체 네오픽셀 파란색으로 한번더 변환
  sendCommand("page pgItemOpen");       // Nextion의 페이지 pgItemOpen으로 변환
  delay(10);
  sendCommand("wOutTagged.en=1");       // Nextion에서 아박 열리는 효과음 재생
  ExpSend();                            // 할당받은 EXP양 UI설정을 위해 Nextion으로 전송
  BatteryPackSend();                    // 할당받은 배터리팩양 UI설정을 위해 Nextion으로 전송
  BoxOpen();                            // 아박 오픈(리니어 모터 ON)
  BlinkTimer.deleteTimer(blinkTimerId); // 전에 사용된 BlinkTimer를 초기화하고 다시 시작하기 위해 종료
  BlinkTimerStart(INNER, YELLOW);       // 내부태그 네오픽셀 노란색 점멸 시작
  GameTimer.deleteTimer(gameTimerId);   // Puzzle함수 -> PuzzleSolved함수 진행되면 이후로는 Activate로 초기화 되지 않게 타이머 종료(기획대로)
  ptrCurrentMode = RfidLoopInner;       // ptr함수의 주소를 RFIDOuter -> RfidInner로 교체 (내부태그하여 아이템가져가기 위해)
  ptrRfidMode = ItemTook;               // 내부태그되고 CheckingPlayer 함수가 실행되면 ItemTook로 실행되게 주소 변경
}

/**
 * @brief PuzzleSolved 함수 실행후 내부 RIFD태그 되어있을때 실행되는 함수 (배터리팩이랑 경험치 가져오는 버그)
 */
void ItemTook()
{
  /* #region  배터리팩 개수 Serial로 확인하는 부분 */
  Serial.println("ItemTook");
  Serial.println(((int)tag["battery_pack"] + (int)my["battery_pack"]));
  Serial.println((int)my["max_battery_pack"]);
  /* #endregion */
  if (((int)tag["battery_pack"] + (int)my["battery_pack"]) <= (int)tag["max_battery_pack"]){                                    // 태그한 플레이어의 현재 배터리팩 최대 소지 가능 개수가 >= 아이템박스에서 얻을 수 있는거 보다 많거나 같을때
    sendCommand("page pgItemTaken");                                                                                            // Nextion에서 배터리팩 가져간 후 페이지로 변경 + 효과음은 페이지 pgItemTakenb 변경시 nextion에서 자동재생
    AllNeoOn(RED);                                                                                                              // 가져가고 나서 USED일땐 전체 빨간색
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "used");                                             // 아박 device_state = used 처리
    has2wifi.Send((String)(const char *)tag["device_name"], "battery_pack", ("+" + (String)(const char *)my["battery_pack"]));  // 태그한 플레이어 배터리팩 개수 추가
    has2wifi.Send((String)(const char *)tag["device_name"], "exp", ("+" + (String)(const char *)my["exp_pack"]));               // 태그한 플레이어 경험치 추가
    has2wifi.Send((String)(const char *)my["device_name"], "battery_pack", ("-" + (String)(const char *)my["battery_pack"]));   // 태그된 아박 배터리팩 개수 감소
    has2wifi.Send((String)(const char *)my["device_name"], "exp_pack", ("-" + (String)(const char *)my["exp_pack"]));           // 태그된 아박 경험치 감소
    BlinkTimer.deleteTimer(blinkTimerId);                                                                                       // 내부태그 황색 점멸  종료
    itemBoxUsed = true;                                                                                                         // used 명령 들어와도 재실행 되지 않게 제한하는 bool 변수
    ptrCurrentMode = WaitFunc;                                                                                                  // ptr 함수의 실행이 null로 변환
    ptrRfidMode = WaitFunc;                                                                                                     // ptr 함수의 실행이 null로 변환
  }
  else                                                  // 태그한 플레이어가 더이상 배터리팩을 소지할 수 없을 때 실행 
  {
    Serial.println("NOT ENOUGH IOT BatteryPack");       //
    sendCommand("page pgItemTakeFail");                 // Nextion에서 더이상 소지할수 없다는 안내창과 효과음 출력을 위해 serial 전송
    NeoBlink(INNER, RED, 4, 250);                       // 내부 네오픽셀 4번 0.25s 간격으로 적색 점멸 -> Delay사용으로 이 함수에 2초 머물러 있음
    BlinkTimer.deleteTimer(blinkTimerId);               // 내부 네오픽셀 황색 점멸 타이머 초기화를 위해 종료
    BlinkTimerStart(INNER, YELLOW);                     // 내부 네오픽셀 황색 점멸 타이머 시작
  }
}
#line 1 "c:\\Users\\HAS1\\Desktop\\BBangJun\\Final_Code\\itembox_random_add\\timer.ino"
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
