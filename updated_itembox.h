#ifndef _DONE_ITEMBOX_CODE_
#define _DONE_ITEMBOX_CODE_
#include "Library_and_pin.h"
#include "ItemBoxState.h"
const int rfid_num = 2;
ItemBoxState currentState = ItemBoxState::BOOT;
//****************************************WIFI****************************************************************
HAS2_Wifi has2wifi("http://172.30.1.43");;
void DataChanged();
void ProcessServerStateChanges();
void SettingFunc(void);
void ActivateFunc(void);
void ReadyFunc(void);
void OpenFunc(void);
void WaitFunc(void);
void WifiIntervalLoop(unsigned long intervalValue);
unsigned long wifiInterval = 0;
//****************************************Game System****************************************************************
void WaitFunc(void);                    // forward declaration
void (*ptrCurrentMode)() = WaitFunc;
void (*ptrRfidMode)() = WaitFunc;
//****************************************Game System****************************************************************
unsigned long puzzleStartTime = 0; 
enum {VIBESTREGNTH = 0, ANSWER, RANGE};
enum {ANSWER_CNT = 0, ANSWER_RANGE, VIBRATION_RANGE};
int modeValue[3][5] = { {200,150,100,50,0},
                         {13,43,21,0,0},
                         {3,2,5,0,0}};
int answerCnt = 0;
bool itemBoxSelfOpen = false;
bool itemBoxUsed = false;
enum {HI1=0,HI2,OI1,OI2,BI1,BI2,GI1,GI2,FI1,FI2};
struct BATTERYPACKRND
{
    String strDevice;
    int nVal;
};
struct BATTERYPACKRND batteryPackRnd[10] = {    {"HI1", 0},{"HI2", 0},
                                                {"OI1", 0},{"OI2", 0},
                                                {"BI1", 0},{"BI2", 0},
                                                {"GI1", 0},{"GI2", 0},
                                                {"FI1", 0},{"FI2", 0} };
//****************************************SimpleTimer SETUP****************************************************************
SimpleTimer GameTimer;
SimpleTimer WifiTimer;
SimpleTimer BlinkTimer;
void TimerInit();
void WifiIntervalFunc();
void GameTimerFunc();
void BlinkTimerFunc();
void BlinkTimerStart(int Neo, int NeoColor);
int blinkNeo = 0;
int blinkColor = 0;
bool blinkOn = false;
int wifiTimerId;
int gameTimerId;
int blinkTimerId;
unsigned long wifiTime = 2000;    // 1sec
unsigned long gameTime = 5000;    // 3sec
unsigned long blinkTime = 1300;   // 1sec
volatile unsigned int nGameTimerCnt = 0;
//****************************************Nextion SETUP****************************************************************
HardwareSerial nexHwSerial(2);
String nextion_str = "";
int currner_page_num;
void NextionInit();
void DisplayCheck();
void NextionReceived();
void ExpSend();
void BatteryPackSend();
//****************************************Neopixel SETUP****************************************************************
void NeopixelInit();
void lightColor(Adafruit_NeoPixel &strip, int colorArr[3]);
void EncoderNeopixelOn();
void NeoBlink(int neo, int neoColor, int cnt, int blinkTime);
const int NumPixels[3] = {28,24,24};
const int NeopixelNum = 3;
enum {PN532 = 0, INNER, ENCODER};
enum {WHITE = 0, RED, YELLOW, GREEN, BLUE, PURPLE, BLACK, BLUE0, BLUE1, BLUE2, BLUE3};
int color[11][3] = {    {20, 20, 20},   //WHITE
                        {40, 0, 0},     //RED        
                        {40, 40, 0},    //YELLOW
                        {0, 40, 0},     //GREEN
                        {0, 0, 40},     //BLUE
                        {40, 0, 40},    //PURPLE
                        {0, 0, 0},      //BLACK
                        {0, 0, 20},     //ENCODERBLUE0
                        {0, 0, 40},     //ENCODERBLUE1
                        {0, 0, 60},     //ENCODERBLUE2
                        {0, 0, 80}};    //ENCODERBLUE3
const int neopixel_num = 3;
Adafruit_NeoPixel pixels[NeopixelNum] = {Adafruit_NeoPixel(NumPixels[PN532], PN532_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800),
                                         Adafruit_NeoPixel(NumPixels[ENCODER], ENCODER_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800),
                                         Adafruit_NeoPixel(NumPixels[INNER], INNER_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800)};
//****************************************RFID SETUP****************************************************************
enum {OUTPN532 = 0, INPN532};
Adafruit_PN532 nfc[2] = {Adafruit_PN532(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS1),
                                Adafruit_PN532(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS2)};
bool rfid_tag;
bool rfid_timer_assess;
bool rfid_init_complete[2];
extern bool rfid_init_attempted;
extern unsigned long lastRfidAckMs;
extern unsigned long lastRfidTagMs;
void RfidInit(void);
void RfidLoopInner();
void RfidLoopOutter();
void CheckingPlayers(uint8_t rfidData[32]);
void StartPuzzle();
void PuzzleSolved();
void ItemTook();
//****************************************MOTOR SETUP****************************************************************
const int MotorFreq = 5000;
const int MotorResolution = 8;
const int MotorLedChannel = 3;
const int VibrationLedChannel = 4;
const int MotorMAX_DUTY_CYCLE = (int)(pow(2, MotorResolution) - 1);
const int VibtationMAX_DUTY_CYCLE = (int)(pow(2, MotorResolution) - 1);
void MotorInit();
void MotorUp();
void MotorDown();
void MotorStop();
void MotorService();
void enqueueReadyBatterySends();
void ReadySendService();
void EncoderVibrationStrength(int answer);
enum class MotorAction { IDLE = 0, OPENING, CLOSING };
MotorAction motorAction = MotorAction::IDLE;
unsigned long motorActionStartMs = 0;
const unsigned long motorRunMs = 4000UL;
bool readySendPending = false;
int readySendIndex = HI1;
unsigned long readySendLastMs = 0;
const unsigned long readySendIntervalMs = 120UL;
const bool enableReadyPackSync = false;
//****************************************ENCODER SETUP****************************************************************
void EncoderInit();
long readEncoderValue(void);
void updateEncoder();
volatile int lastEncoded = 0;
volatile long encoderValue = 165;
long lastencoderValue = 0;
int lastMSB = 0;
int lastLSB = 0;
#endif
