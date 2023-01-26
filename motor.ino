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
