void NextionInit()
{
   nexInit();
   nexHwSerial.begin(9600, SERIAL_8N1, NEXTIONHWSERIAL_RX_PIN, NEXTIONHWSERIAL_TX_PIN);
}

void SendLanguage()
{
    int lang = ((String)(const char*)shift_machine["selected_language"] == "EN") ? 0 : 1;
    const char* pages[] = {"pgSetting", "pgItemOpen", "pgItemTaken", "pgItemFail",
                           "pgEscapeOpen", "pgSurvivorWin", "pgSurvivorLose"};
    for (int i = 0; i < 7; i++) {
        String cmd = String(pages[i]) + ".vLanguage.val=" + String(lang);
        sendCommand(cmd.c_str());
    }
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
        sendCommand("pgItemOpen.picExp.pic=0");
        sendCommand("pgItemOpen.vExp.val=10");
    }
    else if((String)(const char*)my["exp_pack"] == "30")
    {
        sendCommand("pgItemOpen.picExp.pic=1");
        sendCommand("pgItemOpen.vExp.val=30");
    }
    else if((String)(const char*)my["exp_pack"] == "50")
    {
        sendCommand("pgItemOpen.picExp.pic=2");
        sendCommand("pgItemOpen.vExp.val=50");
    }
}
void BatteryPackSend(){
    int val = String((const char*)my["battery_pack"]).toInt();
    if (val <= 0) return;
    bool isEN = ((String)(const char*)shift_machine["selected_language"] == "EN");
    int pic = isEN ? (val - 1) : (val + 3);
    String picCmd = "pgItemOpen.picBatteryPack.pic=" + String(pic);
    String valCmd = "pgItemOpen.vBatteryPack.val=" + String(val);
    sendCommand(picCmd.c_str());
    sendCommand(valCmd.c_str());
}

// void BatteryPackSend(){
//     if((String)(const char*)my["battery_pack"] == "1")
//     {
//         sendCommand("picBatteryPack.pic=3");
//         sendCommand("pgItemOpen.vBatteryPack.val=1");
//     }
//     else if((String)(const char*)my["battery_pack"] == "2")
//     {
//         sendCommand("picBatteryPack.pic=4");
//         sendCommand("pgItemOpen.vBatteryPack.val=2");
//     }
//     else if((String)(const char*)my["battery_pack"] == "3")
//     {
//         sendCommand("picBatteryPack.pic=5");
//         sendCommand("pgItemOpen.vBatteryPack.val=3");
//     }
//     else if((String)(const char*)my["battery_pack"] == "4")
//     {
//         sendCommand("picBatteryPack.pic=6");
//         sendCommand("pgItemOpen.vBatteryPack.val=4");
//     }
//     else if((String)(const char*)my["battery_pack"] == "5")
//     {
//         sendCommand("picBatteryPack.pic=7");
//         sendCommand("pgItemOpen.vBatteryPack.val=5");
//     }
// }
