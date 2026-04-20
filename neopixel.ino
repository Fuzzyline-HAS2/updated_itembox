void lightColor(Adafruit_NeoPixel &neo, int c[3])
{
    neo.fill(neo.Color(c[0], c[1], c[2]));
    neo.show();
}

void NeopixelInit()
{
  for (int i = 0; i < NeopixelNum; ++i)
  {
    pixels[i].begin();
    pixels[i].setBrightness(ledBrightness);
  }
  for (int i = 0; i < NeopixelNum; ++i)
  {
    lightColor(pixels[i], color[WHITE]);
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
    lightColor(pixels[neo], color[BLACK]); //전체 off
    delay(blinkTime);
    lightColor(pixels[neo], color[neoColor]); //전체 적색on
    delay(blinkTime);                   //전체 적색on
  }
}

void AllNeoOn(int neoColor){
  for (int i = 0; i < NeopixelNum; ++i)
    lightColor(pixels[i], color[neoColor]);
}

// 서버로부터 brightness(0~100) 수신 → 0~255 매핑 후 픽셀에 적용
// 0이거나 100 초과값 수신 시 기본값 255 사용
void UpdateBrightness() {
    int serverBrightness = my["brightness"].as<int>();
    if (serverBrightness <= 0 || serverBrightness > 100) {
        ledBrightness = DEFAULT_BRIGHTNESS;
    } else {
        ledBrightness = map(serverBrightness, 1, 100, 1, 255);
    }
    for (int i = 0; i < NeopixelNum; i++) {
        pixels[i].setBrightness(ledBrightness);
        pixels[i].show();
    }
}
