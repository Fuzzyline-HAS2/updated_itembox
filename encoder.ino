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
