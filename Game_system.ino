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
