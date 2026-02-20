void MotorInit()
{
    // Linear Motor Init
    pinMode(BOXSWITCH_PIN, INPUT_PULLUP);
    pinMode(MOTOR_INA1_PIN, OUTPUT);
    pinMode(MOTOR_INA2_PIN, OUTPUT);
    ledcAttach(MOTOR_PWMA_PIN, MotorFreq, MotorResolution);
    ledcWrite(MOTOR_PWMA_PIN, 0);
    BoxClose();

    // Vibration Motor Init
    pinMode(VIBRATION_ANSWER_PIN, OUTPUT);
    digitalWrite(VIBRATION_ANSWER_PIN, HIGH);
    ledcAttach(VIBRATION_RANGE_PIN, MotorFreq, MotorResolution);
    ledcWrite(VIBRATION_RANGE_PIN, 0);
    MotorStop();
}

void BoxClose()
{
    Serial.println("BOX Close");
    ledcWrite(MOTOR_PWMA_PIN, MotorMAX_DUTY_CYCLE - 1);
    digitalWrite(MOTOR_INA1_PIN, LOW);
    digitalWrite(MOTOR_INA2_PIN, HIGH);
    motorAction = MotorAction::CLOSING;
    motorActionStartMs = millis();
}

void BoxOpen()
{
    Serial.println("BOX Open");
    ledcWrite(MOTOR_PWMA_PIN, MotorMAX_DUTY_CYCLE - 1);
    digitalWrite(MOTOR_INA1_PIN, HIGH);
    digitalWrite(MOTOR_INA2_PIN, LOW);
    motorAction = MotorAction::OPENING;
    motorActionStartMs = millis();
}

void MotorStop()
{
    Serial.println("Motor Stop");
    digitalWrite(MOTOR_INA1_PIN, LOW);
    digitalWrite(MOTOR_INA2_PIN, LOW);
    ledcWrite(MOTOR_PWMA_PIN, 0);
    motorAction = MotorAction::IDLE;
}

void MotorService()
{
    if (motorAction == MotorAction::IDLE) {
        return;
    }

    if ((millis() - motorActionStartMs) >= motorRunMs) {
        MotorAction completedAction = motorAction;
        MotorStop();
        if (completedAction == MotorAction::OPENING) {
            Serial.println("BOX Opened");
        } else if (completedAction == MotorAction::CLOSING) {
            Serial.println("BOX Closed");
        }
    }
}

void EncoderVibrationStrength(int answer)
{
    int differenceValue = abs(answer - (encoderValue / 4));
    int answerRange = modeValue[RANGE][1];
    int vibeRange = modeValue[RANGE][2];
    int vibeStrength = 0;
    if (differenceValue < answerRange + vibeRange * 0) vibeStrength = 0;
    else if (differenceValue < answerRange + vibeRange * 1) vibeStrength = 1;
    else if (differenceValue < answerRange + vibeRange * 2) vibeStrength = 2;
    else if (differenceValue < answerRange + vibeRange * 3) vibeStrength = 3;
    else vibeStrength = 4;
    ledcWrite(VIBRATION_RANGE_PIN, modeValue[VIBESTREGNTH][vibeStrength]);
}
