/**
 * @brief ?´ë? ?¸ë? pn532 ì´ˆê¸°?œì„±??ë°??¤íŒ¨???¬ì‹œ??(ìµœë? 10??
 */
bool rfid_init_attempted = false;
unsigned long lastRfidAckMs = 0;
unsigned long lastRfidTagMs = 0;

void RfidInit()
{
  rfid_init_attempted = true;
  const int MAX_RETRY = 10;
  int retryCount = 0;
  bool allConnected = false;

  while (!allConnected && retryCount < MAX_RETRY)
  {
    allConnected = true;
    for (int i = 0; i < rfid_num; ++i)
    {
      nfc[i].begin();
      if (!(nfc[i].getFirmwareVersion()))                 // pn532 ?™ì‘ ?ˆí• ??
      {
        Serial.println("PN532 ?°ê²°?¤íŒ¨ : " + String(i) + " (?¬ì‹œ??" + String(retryCount + 1) + "/" + String(MAX_RETRY) + ")");
        AllNeoOn(RED);
        allConnected = false;
        rfid_init_complete[i] = false;
      }
      else
      {
        nfc[i].SAMConfig();
        Serial.println("PN532 ?°ê²°?±ê³µ : " + String(i));
        rfid_init_complete[i] = true;
        AllNeoOn(YELLOW);
      }
      yield();                                            // Watchdog ?¼ë“œ (ë¸”ë¡œ???†ìŒ)
    }
    if (!allConnected) retryCount++;
  }
  if (!allConnected) {
    Serial.println("? ï¸ PN532 ì´ˆê¸°???¤íŒ¨ - ìµœë? ?¬ì‹œ??ì´ˆê³¼");
  }
}
/**
 * @brief ?„ì´?œë°•???´ë? pn532 ?œê·¸ ?½ì–´?€??CheckingPlayerë¡??„ì†¡
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
  {
    lastRfidAckMs = millis();                                                                           // rfid ?µì‹  ê°€?¥í•œ ?íƒœ?¸ì? ?•ì¸
    if (nfc[INPN532].startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A))
    {                                                                         // rfid??tag ì°í˜”?”ì? ?•ì¸??//?°ì´???¤ì–´?¤ë©´ uid?•ë³´ ê°€?¸ì˜¤ê¸?
      if (nfc[INPN532].ntag2xx_ReadPage(7, data))
      {
        lastRfidTagMs = millis();                                                                       // ntag ?°ì´?°ì— ?‘ê·¼?´ì„œ ë¶ˆëŸ¬?€??data?‰ì—´???€??
        Serial.println("TAGGGED");
        CheckingPlayers(data);
      }
    }
  }
  // TODO InnerRFID ë£¨í”„???°ê²° ?ˆë˜ë©??Œì¹˜??
  // else(
  //   ESP.restart();
  // )
}
/**
 * @brief ?„ì´?œë°•???¸ë? pn532 ?œê·¸ ?½ì–´?€??CheckingPlayerë¡??„ì†¡
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
  {
    lastRfidAckMs = millis();                                                                           // rfid ?µì‹  ê°€?¥í•œ ?íƒœ?¸ì? ?•ì¸
    if (nfc[OUTPN532].startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A))
    {                                                                         // rfid??tag ì°í˜”?”ì? ?•ì¸??//?°ì´???¤ì–´?¤ë©´ uid?•ë³´ ê°€?¸ì˜¤ê¸?
      if (nfc[OUTPN532].ntag2xx_ReadPage(7, data))
      {
        lastRfidTagMs = millis();                                                                       // ntag ?°ì´?°ì— ?‘ê·¼?´ì„œ ë¶ˆëŸ¬?€??data?‰ì—´???€??
        Serial.println("TAGGGED");
        CheckingPlayers(data);
      }
    }
  }
  // TODO OutterRFID ë£¨í”„???°ê²° ?ˆë˜ë©??Œì¹˜??
  // else(
  //   ESP.restart();
  // )
}

/**
 * @brief ?´ì™¸ë¶€?ì„œ ?œê·¸??ì¹´ë“œ?°ì´??string?¼ë¡œ ë³€?˜í›„ DB???”ì²­?˜ì—¬ 'role'?•ì¸?˜ì—¬ ptrRfidModeë¡??„ì†¡
 */
void CheckingPlayers(uint8_t rfidData[32])                // ?´ë–¤ ì¹´ë“œê°€ ?¤ì–´?”ëŠ”ì§€ ?•ì¸??
{
  String tagUser = "";                                    // ?½ì–´??uint_8tê°?string?¼ë¡œ ë³€?˜í•˜ê¸??„í•œ String ë³€??
  for (int i = 0; i < 4; i++)                             // GxPx ?°ì´?°ë§Œ ë°°ì—´?ì„œ ì¶”ì¶œ?´ì„œ string?¼ë¡œ ?€??
    tagUser += (char)rfidData[i];
  Serial.println("tag_user_data : " + tagUser);
  if (tagUser == "MMMM")
  {                                                       //"MMMM"?¼ê²½??DB?”ì²­ ?˜ì? ?Šê³  ë°”ë¡œ watchdog ?¤í–‰(DB??MMMM ?Œë ˆ?´ì–´??ì¡´ì¬?˜ì? ?Šì•„???”ì²­?˜ë©´ ?¤ë¥˜ ë°œìƒ)
    ESP.restart();
  }
                                                          // 1. ?œê·¸???Œë ˆ?´ì–´????• ê³??ëª…ì¹©ê°¯?? ìµœë??ëª…ì¹©ê°¯?????½ì–´?¤ê¸°
  has2wifi.Receive(tagUser);                              // 2. ? ë˜?¸ì?, ?Œë ˆ?´ì–´?¸ì? êµ¬ë¶„
  if ((String)(const char *)tag["role"] == "player")      // 3. ?œê·¸???¬ìš©?ê? ?Œë ˆ?´ì–´ ?´ë©´
  { 
    Serial.println("Player Tagged");
    ptrRfidMode();
  }
  else if ((String)(const char *)tag["role"] == "tagger") // 4. ?œê·¸???¬ìš©?ê? ? ë˜ë©??„ë¬´ ë³€??x
    Serial.println("Tagger Tagged");
  else if ((String)(const char *)tag["role"] == "ghost")  // 5. ?œê·¸???¬ìš©?ê? ? ë ¹?´ë©´ ?„ë¬´ ë³€??x
    Serial.println("Ghost Tagged");
  else // 6. ?ˆì™¸ ì²˜ë¦¬
    Serial.println("Wrong TAG");
}

/**
 * @brief Activate ?í™©?ì„œ ?¸ë? pn532 ?œê·¸???”ì½”???œì„±????RFID ì¤‘ì? Puzzle?¨ìˆ˜ ?¤í–‰
 */
void StartPuzzle()
{
  transitionTo(ItemBoxState::PUZZLE);
  Serial.println("StartPuzzle");
  WifiTimer.deleteTimer(wifiTimerId);                           // ?”ì½”???‰ì„ ?†ì• ê¸??„í•´ wifi read ?”ì½”???¬ìš©?™ì•ˆ??ì¤‘ë‹¨
  GameTimer.deleteTimer(gameTimerId);
  gameTimerId = GameTimer.setInterval(gameTime, GameTimerFunc); // puzzle ?¨ìˆ˜ ?¤í–‰?™ì•ˆ nì´??´ìƒ ?…ë ¥???†ìœ¼ë©?ì´ˆê¸°???˜ê¸°?„í•´ ?œê°„???¬ëŠ” ?€?´ë¨¸ ?œì‘
  answerCnt = 0;
  ptrCurrentMode = Puzzle;                                      // ptr?¨ìˆ˜??ì£¼ì†Œë¥?RFIDOuter -> Puzzleë¡?ë³€??
  AllNeoOn(BLUE);                                               // puzzle ?¨ìˆ˜ ì§„í–‰?™ì•ˆ ?„ì²´ ?¤ì˜¤?½ì? ?Œë???? ì?
  attachInterrupt(encoderPinA, updateEncoder, CHANGE);          // ?”ì½”???˜ë“œ?¨ì–´ ?¸í„°?½íŠ¸ ?œì„±??
  attachInterrupt(encoderPinB, updateEncoder, CHANGE);
}

/**
 * @brief Puzzle?¨ìˆ˜ë¡?ë¬¸ì œë¥???ë§ì¶˜ ???„ë£Œ?˜ëŠ” ?œê·¸ë¥??¤í–‰?ˆì„???¤í–‰?˜ëŠ” ?¨ìˆ˜
 */
void PuzzleSolved()
{
  transitionTo(ItemBoxState::OPEN);
  itemBoxSelfOpen = true;                                                         // ?œê·¸?˜ë©´ ?„ì´?œë°•?¤ê? open ?íƒœ ?„ìœ¼ë¡?ë©”ì¸?ì„œ open ëª…ë ¹ ?¤ì–´?€???¬ì‹¤?‰ë˜ì§€ ?Šê²Œ ?œí•œ?˜ëŠ” bool ë³€??
  has2wifi.Send((String)(const char *)my["device_name"], "device_state", "open"); // ë©”ì¸?¼ë¡œ ?„ì¬ë¶€???„ë°•???íƒœê°€ open?¼ë¡œ ?€??
  Serial.println("PuzzleSolved");
  AllNeoOn(BLUE);                       // ?”ì½”???¤ì˜¤?½ì???ë¹¨ê°„???¬ì¸?€ ?†ì• ê¸??„í•´ ?„ì²´ ?¤ì˜¤?½ì? ?Œë??‰ìœ¼ë¡??œë²ˆ??ë³€??
  sendCommand("page pgItemOpen");       // Nextion???˜ì´ì§€ pgItemOpen?¼ë¡œ ë³€??
  delay(10);
  sendCommand("wOutTagged.en=1");       // Nextion?ì„œ ?„ë°• ?´ë¦¬???¨ê³¼???¬ìƒ
  ExpSend();                            // ? ë‹¹ë°›ì? EXP??UI?¤ì •???„í•´ Nextion?¼ë¡œ ?„ì†¡
  BatteryPackSend();                    // ? ë‹¹ë°›ì? ë°°í„°ë¦¬íŒ©??UI?¤ì •???„í•´ Nextion?¼ë¡œ ?„ì†¡
  BoxOpen();                            // ?„ë°• ?¤í”ˆ(ë¦¬ë‹ˆ??ëª¨í„° ON)
  BlinkTimer.deleteTimer(blinkTimerId); // ?„ì— ?¬ìš©??BlinkTimerë¥?ì´ˆê¸°?”í•˜ê³??¤ì‹œ ?œì‘?˜ê¸° ?„í•´ ì¢…ë£Œ
  BlinkTimerStart(INNER, YELLOW);       // ?´ë??œê·¸ ?¤ì˜¤?½ì? ?¸ë????ë©¸ ?œì‘
  GameTimer.deleteTimer(gameTimerId);   // Puzzle?¨ìˆ˜ -> PuzzleSolved?¨ìˆ˜ ì§„í–‰?˜ë©´ ?´í›„ë¡œëŠ” Activateë¡?ì´ˆê¸°???˜ì? ?Šê²Œ ?€?´ë¨¸ ì¢…ë£Œ(ê¸°íš?€ë¡?
  ptrCurrentMode = RfidLoopInner;       // ptr?¨ìˆ˜??ì£¼ì†Œë¥?RFIDOuter -> RfidInnerë¡?êµì²´ (?´ë??œê·¸?˜ì—¬ ?„ì´?œê??¸ê?ê¸??„í•´)
  ptrRfidMode = ItemTook;               // ?´ë??œê·¸?˜ê³  CheckingPlayer ?¨ìˆ˜ê°€ ?¤í–‰?˜ë©´ ItemTookë¡??¤í–‰?˜ê²Œ ì£¼ì†Œ ë³€ê²?
}

/**
 * @brief PuzzleSolved ?¨ìˆ˜ ?¤í–‰???´ë? RIFD?œê·¸ ?˜ì–´?ˆì„???¤í–‰?˜ëŠ” ?¨ìˆ˜ (ë°°í„°ë¦¬íŒ©?´ë‘ ê²½í—˜ì¹?ê°€?¸ì˜¤??ë²„ê·¸)
 */
void ItemTook()
{
  /* #region  ë°°í„°ë¦¬íŒ© ê°œìˆ˜ Serialë¡??•ì¸?˜ëŠ” ë¶€ë¶?*/
  Serial.println("ItemTook");
  Serial.println(((int)tag["battery_pack"] + (int)my["battery_pack"]));
  Serial.println((int)my["max_battery_pack"]);
  /* #endregion */
  if (((int)tag["battery_pack"] + (int)my["battery_pack"]) <= (int)tag["max_battery_pack"]){
    transitionTo(ItemBoxState::USED);                                    // ?œê·¸???Œë ˆ?´ì–´???„ì¬ ë°°í„°ë¦¬íŒ© ìµœë? ?Œì? ê°€??ê°œìˆ˜ê°€ >= ?„ì´?œë°•?¤ì—???»ì„ ???ˆëŠ”ê±?ë³´ë‹¤ ë§ê±°??ê°™ì„??
    sendCommand("page pgItemTaken");                                                                                            // Nextion?ì„œ ë°°í„°ë¦¬íŒ© ê°€?¸ê°„ ???˜ì´ì§€ë¡?ë³€ê²?+ ?¨ê³¼?Œì? ?˜ì´ì§€ pgItemTakenb ë³€ê²½ì‹œ nextion?ì„œ ?ë™?¬ìƒ
    AllNeoOn(RED);                                                                                                              // ê°€?¸ê?ê³??˜ì„œ USED?¼ë• ?„ì²´ ë¹¨ê°„??
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "used");                                             // ?„ë°• device_state = used ì²˜ë¦¬
    has2wifi.Send((String)(const char *)tag["device_name"], "battery_pack", ("+" + (String)(const char *)my["battery_pack"]));  // ?œê·¸???Œë ˆ?´ì–´ ë°°í„°ë¦¬íŒ© ê°œìˆ˜ ì¶”ê?
    has2wifi.Send((String)(const char *)tag["device_name"], "exp", ("+" + (String)(const char *)my["exp_pack"]));               // ?œê·¸???Œë ˆ?´ì–´ ê²½í—˜ì¹?ì¶”ê?
    has2wifi.Send((String)(const char *)my["device_name"], "battery_pack", ("-" + (String)(const char *)my["battery_pack"]));   // ?œê·¸???„ë°• ë°°í„°ë¦¬íŒ© ê°œìˆ˜ ê°ì†Œ
    has2wifi.Send((String)(const char *)my["device_name"], "exp_pack", ("-" + (String)(const char *)my["exp_pack"]));           // ?œê·¸???„ë°• ê²½í—˜ì¹?ê°ì†Œ
    BlinkTimer.deleteTimer(blinkTimerId);                                                                                       // ?´ë??œê·¸ ?©ìƒ‰ ?ë©¸  ì¢…ë£Œ
    itemBoxUsed = true;                                                                                                         // used ëª…ë ¹ ?¤ì–´?€???¬ì‹¤???˜ì? ?Šê²Œ ?œí•œ?˜ëŠ” bool ë³€??
    ptrCurrentMode = WaitFunc;                                                                                                  // ptr ?¨ìˆ˜???¤í–‰??nullë¡?ë³€??
    ptrRfidMode = WaitFunc;                                                                                                     // ptr ?¨ìˆ˜???¤í–‰??nullë¡?ë³€??
  }
  else                                                  // ?œê·¸???Œë ˆ?´ì–´ê°€ ?”ì´??ë°°í„°ë¦¬íŒ©???Œì??????†ì„ ???¤í–‰ 
  {
    Serial.println("NOT ENOUGH IOT BatteryPack");       //
    sendCommand("page pgItemTakeFail");                 // Nextion?ì„œ ?”ì´???Œì?? ìˆ˜ ?†ë‹¤???ˆë‚´ì°½ê³¼ ?¨ê³¼??ì¶œë ¥???„í•´ serial ?„ì†¡
    NeoBlink(INNER, RED, 4, 250);                       // ?´ë? ?¤ì˜¤?½ì? 4ë²?0.25s ê°„ê²©?¼ë¡œ ?ìƒ‰ ?ë©¸ -> Delay?¬ìš©?¼ë¡œ ???¨ìˆ˜??2ì´?ë¨¸ë¬¼???ˆìŒ
    BlinkTimer.deleteTimer(blinkTimerId);               // ?´ë? ?¤ì˜¤?½ì? ?©ìƒ‰ ?ë©¸ ?€?´ë¨¸ ì´ˆê¸°?”ë? ?„í•´ ì¢…ë£Œ
    BlinkTimerStart(INNER, YELLOW);                     // ?´ë? ?¤ì˜¤?½ì? ?©ìƒ‰ ?ë©¸ ?€?´ë¨¸ ?œì‘
  }
}

