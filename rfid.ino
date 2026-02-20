/**
 * @brief Initialize both PN532 modules with bounded retries.
 */
bool rfid_init_attempted = false;
unsigned long lastRfidAckMs = 0;
unsigned long lastRfidTagMs = 0;
static unsigned long lastRfidUnavailableLogMs = 0;
static const uint16_t RFID_ACK_TIMEOUT_MS = 60;
void RfidInit()
{
  rfid_init_attempted = true;
  const int MAX_RETRY = 10;
  int retryCount = 0;
  bool allConnected = false;
  Serial.println("[RFID] init start");
  while (!allConnected && retryCount < MAX_RETRY)
  {
    Serial.println("[RFID] init retry " + String(retryCount + 1) + "/" + String(MAX_RETRY));
    allConnected = true;
    for (int i = 0; i < rfid_num; ++i)
    {
      Serial.println("[RFID] begin(" + String(i) + ") start");
      nfc[i].begin();
      Serial.println("[RFID] begin(" + String(i) + ") done");

      uint32_t fw = nfc[i].getFirmwareVersion();
      if (!fw)
      {
        Serial.println("PN532 init failed: " + String(i) + " (retry " + String(retryCount + 1) + "/" + String(MAX_RETRY) + ")");
        AllNeoOn(RED);
        allConnected = false;
        rfid_init_complete[i] = false;
      }
      else
      {
        Serial.printf("[RFID] fw[%d]=0x%08lX\n", i, (unsigned long)fw);
        nfc[i].SAMConfig();
        Serial.println("PN532 init ok: " + String(i));
        rfid_init_complete[i] = true;
        AllNeoOn(YELLOW);
      }
      yield();
    }
    if (!allConnected) retryCount++;
  }
  if (!allConnected) {
    Serial.println("One or more PN532 modules failed to initialize");
  }
  Serial.println("[RFID] init end");
}
/**
 * @brief Read inner RFID and dispatch decoded tag data.
 */
void RfidLoopInner()
{
  if (!rfid_init_complete[INPN532]) {
    unsigned long now = millis();
    if (now - lastRfidUnavailableLogMs >= 5000UL) {
      Serial.println("RFID inner unavailable, skipping read loop");
      lastRfidUnavailableLogMs = now;
    }
    yield();
    return;
  }

  uint8_t uid[3][7] = {{0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0, 0, 0}}; // Buffer to store the returned UID
  uint8_t uidLength[] = {0};                   // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t data[32];
  byte pn532_packetbuffer11[64];
  pn532_packetbuffer11[0] = 0x00;
  if (nfc[INPN532].sendCommandCheckAck(pn532_packetbuffer11, 1, RFID_ACK_TIMEOUT_MS))
  {
    lastRfidAckMs = millis();
    if (nfc[INPN532].startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A))
    {
      if (nfc[INPN532].ntag2xx_ReadPage(7, data))
      {
        lastRfidTagMs = millis();
        Serial.println("TAGGGED");
        CheckingPlayers(data);
      }
    }
  }
  yield();
  // else(
  //   ESP.restart();
  // )
}
/**
 * @brief Read outer RFID and dispatch decoded tag data.
 */
void RfidLoopOutter()
{
  if (!rfid_init_complete[OUTPN532]) {
    unsigned long now = millis();
    if (now - lastRfidUnavailableLogMs >= 5000UL) {
      Serial.println("RFID outer unavailable, skipping read loop");
      lastRfidUnavailableLogMs = now;
    }
    yield();
    return;
  }

  uint8_t uid[3][7] = {{0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 0, 0, 0}}; // Buffer to store the returned UID
  uint8_t uidLength[] = {0};                   // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t data[32];
  byte pn532_packetbuffer11[64];
  pn532_packetbuffer11[0] = 0x00;
  if (nfc[OUTPN532].sendCommandCheckAck(pn532_packetbuffer11, 1, RFID_ACK_TIMEOUT_MS))
  {
    lastRfidAckMs = millis();
    if (nfc[OUTPN532].startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A))
    {
      if (nfc[OUTPN532].ntag2xx_ReadPage(7, data))
      {
        lastRfidTagMs = millis();
        Serial.println("TAGGGED");
        CheckingPlayers(data);
      }
    }
  }
  yield();
  // else(
  //   ESP.restart();
  // )
}
/**
 * @brief Parse tag payload and route to the active RFID handler.
 */
void CheckingPlayers(uint8_t rfidData[32])
{
  String tagUser = "";
  for (int i = 0; i < 4; i++)
    tagUser += (char)rfidData[i];
  Serial.println("tag_user_data : " + tagUser);
  if (tagUser == "MMMM")
  {
    ESP.restart();
  }
  has2wifi.Receive(tagUser);
  if ((String)(const char *)tag["role"] == "player")
  { 
    Serial.println("Player Tagged");
    ptrRfidMode();
  }
  else if ((String)(const char *)tag["role"] == "tagger")
    Serial.println("Tagger Tagged");
  else if ((String)(const char *)tag["role"] == "ghost")
    Serial.println("Ghost Tagged");
  else
    Serial.println("Wrong TAG");
}
/**
 * @brief Enter puzzle mode after valid outer RFID tag.
 */
void StartPuzzle()
{
  transitionTo(ItemBoxState::PUZZLE);
  Serial.println("StartPuzzle");
  WifiTimer.deleteTimer(wifiTimerId);
  GameTimer.deleteTimer(gameTimerId);
  gameTimerId = GameTimer.setInterval(gameTime, GameTimerFunc);
  answerCnt = 0;
  ptrCurrentMode = Puzzle;
  AllNeoOn(BLUE);
  attachInterrupt(encoderPinA, updateEncoder, CHANGE);
  attachInterrupt(encoderPinB, updateEncoder, CHANGE);
}
/**
 * @brief Handle puzzle completion and open box flow.
 */
void PuzzleSolved()
{
  transitionTo(ItemBoxState::OPEN);
  itemBoxSelfOpen = true;
  has2wifi.Send((String)(const char *)my["device_name"], "device_state", "open");
  Serial.println("PuzzleSolved");
  AllNeoOn(BLUE);
  sendCommand("page pgItemOpen");
  delay(10);
  sendCommand("wOutTagged.en=1");
  ExpSend();
  BatteryPackSend();
  BoxOpen();
  BlinkTimer.deleteTimer(blinkTimerId);
  BlinkTimerStart(INNER, YELLOW);
  GameTimer.deleteTimer(gameTimerId);
  ptrCurrentMode = RfidLoopInner;
  ptrRfidMode = ItemTook;
}
/**
 * @brief Handle item pickup and resource transfer flow.
 */
void ItemTook()
{
  /* #region debug */
  Serial.println("ItemTook");
  Serial.println(((int)tag["battery_pack"] + (int)my["battery_pack"]));
  Serial.println((int)my["max_battery_pack"]);
  /* #endregion */
  if (((int)tag["battery_pack"] + (int)my["battery_pack"]) <= (int)tag["max_battery_pack"]){
    transitionTo(ItemBoxState::USED);
    sendCommand("page pgItemTaken");
    AllNeoOn(RED);
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "used");
    has2wifi.Send((String)(const char *)tag["device_name"], "battery_pack", ("+" + (String)(const char *)my["battery_pack"]));
    has2wifi.Send((String)(const char *)tag["device_name"], "exp", ("+" + (String)(const char *)my["exp_pack"]));
    has2wifi.Send((String)(const char *)my["device_name"], "battery_pack", ("-" + (String)(const char *)my["battery_pack"]));
    has2wifi.Send((String)(const char *)my["device_name"], "exp_pack", ("-" + (String)(const char *)my["exp_pack"]));
    BlinkTimer.deleteTimer(blinkTimerId);
    itemBoxUsed = true;
    ptrCurrentMode = WaitFunc;
    ptrRfidMode = WaitFunc;
  }
  else
  {
    Serial.println("NOT ENOUGH IOT BatteryPack");       //
    sendCommand("page pgItemTakeFail");
    NeoBlink(INNER, RED, 4, 250);
    BlinkTimer.deleteTimer(blinkTimerId);
    BlinkTimerStart(INNER, YELLOW);
  }
}
