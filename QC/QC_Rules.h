#ifndef QC_RULES_H
#define QC_RULES_H

#include "QC_Types.h"
#include <HAS2_Wifi.h>
#include <WiFi.h>
#include <cstdint>
#include <esp_system.h>
#include "../ItemBoxState.h"
#include "../Library_and_pin.h"

// 'my' is declared in HAS2_Wifi.h (included above)

// ---------------------------------------------------------
// Rule: Free Heap Memory
// ---------------------------------------------------------
class QCRule_HeapMemory : public IQCRule {
public:
  String getId() const override { return "SYS_MEM_01"; }
  String getName() const override { return "Free Heap Check"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    uint32_t freeHeap = ESP.getFreeHeap();
    String val = String(freeHeap) + " bytes";
    const uint32_t WARN_THRESHOLD = 20000;
    const uint32_t FAIL_THRESHOLD = 10000;
    if (freeHeap < FAIL_THRESHOLD) {
      return QCResult(QCLevel::FAIL, getId(), "System Free Heap",
                      "< " + String(FAIL_THRESHOLD), val,
                      "Check for memory leaks or reduce static allocation");
    } else if (freeHeap < WARN_THRESHOLD) {
      return QCResult(QCLevel::WARN, getId(), "System Free Heap",
                      "< " + String(WARN_THRESHOLD), val,
                      "Monitor memory usage closely");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: WiFi Signal Strength
// ---------------------------------------------------------
class QCRule_WifiSignal : public IQCRule {
public:
  String getId() const override { return "NET_WIFI_01"; }
  String getName() const override { return "WiFi Signal Strength"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    if (WiFi.status() != WL_CONNECTED) {
      return QCResult();
    }
    int32_t rssi = WiFi.RSSI();
    String val = String(rssi) + " dBm";
    const int32_t WARN_THRESHOLD = -75;
    const int32_t FAIL_THRESHOLD = -85;
    if (rssi < FAIL_THRESHOLD) {
      return QCResult(QCLevel::FAIL, getId(), "WiFi RSSI",
                      "< " + String(FAIL_THRESHOLD), val,
                      "Move device closer to AP or check antenna");
    } else if (rssi < WARN_THRESHOLD) {
      return QCResult(QCLevel::WARN, getId(), "WiFi RSSI",
                      "< " + String(WARN_THRESHOLD), val,
                      "Signal weak, potential packet loss");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: WiFi Connection Status
// ---------------------------------------------------------
class QCRule_WifiConnection : public IQCRule {
public:
  String getId() const override { return "NET_WIFI_00"; }
  String getName() const override { return "WiFi Connection Status"; }
  bool isFastCheck() const override { return true; }

  QCResult check() override {
    if (WiFi.status() != WL_CONNECTED) {
      return QCResult(QCLevel::FAIL, getId(), "WiFi Status", "Connected",
                      "Disconnected",
                      "Check Router/Credentials or Reset Device");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Last Reset Reason (Brownout / WDT / Panic)
// ---------------------------------------------------------
class QCRule_ResetReason : public IQCRule {
public:
  String getId() const override { return "SYS_RST_01"; }
  String getName() const override { return "Reset Reason Check"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    static bool reported = false;
    if (reported) return QCResult();

    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_BROWNOUT) {
      reported = true;
      return QCResult(QCLevel::FAIL, getId(), "Last Reset Reason",
                      "No brownout", "ESP_RST_BROWNOUT",
                      "Check 5V/3.3V rail, motor surge noise, shared GND");
    }
    bool isWdtReset = (reason == ESP_RST_WDT || reason == ESP_RST_INT_WDT);
#ifdef ESP_RST_TASK_WDT
    isWdtReset = isWdtReset || (reason == ESP_RST_TASK_WDT);
#endif
    if (isWdtReset) {
      reported = true;
      return QCResult(QCLevel::WARN, getId(), "Last Reset Reason",
                      "No watchdog reset", String((int)reason),
                      "Check blocking code and peripheral hangs");
    }
    if (reason == ESP_RST_PANIC) {
      reported = true;
      return QCResult(QCLevel::WARN, getId(), "Last Reset Reason",
                      "No panic reset", "ESP_RST_PANIC",
                      "Check null pointer / memory corruption");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Game State Consistency
// ---------------------------------------------------------
class QCRule_GameState : public IQCRule {
public:
  String getId() const override { return "LOGIC_STATE_01"; }
  String getName() const override { return "Game State Consistency"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    // Skip check if 'my' has no data yet (device not registered or backend not
    // responding)
    if (my.size() == 0) {
      return QCResult(); // PASS - no data to validate yet
    }

    if (my.containsKey("game_state")) {
      String gState = (const char *)my["game_state"];
      bool gValid = (gState == "setting" || gState == "activate" ||
                     gState == "ready" || gState == "");
      if (!gValid) {
        return QCResult(QCLevel::WARN, getId(), "game_state Value",
                        "Known State", gState,
                        "Check Logic for invalid state assignment");
      }
    }

    if (my.containsKey("device_state")) {
      String dState = (const char *)my["device_state"];
      bool dValid =
          (dState == "setting" || dState == "activate" || dState == "ready" ||
           dState == "open" || dState == "used" || dState == "close" ||
           dState == "repaired_all" || dState == "player_win" ||
           dState == "player_lose" || dState == "");
      if (!dValid) {
        return QCResult(QCLevel::WARN, getId(), "device_state Value",
                        "Known State", dState,
                        "Check Logic for invalid state assignment");
      }
    }

    if (my.containsKey("manage_state")) {
      String mState = (const char *)my["manage_state"];
      bool mValid = (mState == "mo" || mState == "mu" || mState == "");
      if (!mValid) {
        return QCResult(QCLevel::WARN, getId(), "manage_state Value",
                        "Known State", mState,
                        "Check Logic for invalid state assignment");
      }
    }

    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Dual RFID Initialization Status
// ---------------------------------------------------------
class QCRule_RfidStatus : public IQCRule {
public:
  String getId() const override { return "HW_RFID_01"; }
  String getName() const override { return "Dual RFID Init Status"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    extern bool rfid_init_attempted;
    extern bool rfid_init_complete[2];
    if (!rfid_init_attempted) {
      return QCResult();
    }
    if (!rfid_init_complete[0] || !rfid_init_complete[1]) {
      String val = "Inner:" + String(rfid_init_complete[1]) +
                   " Outer:" + String(rfid_init_complete[0]);
      return QCResult(QCLevel::FAIL, getId(), "RFID Init",
                      "Both initialized", val,
                      "Check PN532 wiring and SPI connections");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: RFID Runtime Heartbeat in RFID-required FSM states
// ---------------------------------------------------------
class QCRule_RfidHeartbeat : public IQCRule {
public:
  String getId() const override { return "HW_RFID_02"; }
  String getName() const override { return "RFID Runtime Heartbeat"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    extern bool rfid_init_attempted;
    extern unsigned long lastRfidAckMs;
    if (!rfid_init_attempted) return QCResult();

    bool needsRfid = (currentState == ItemBoxState::ACTIVATE ||
                      currentState == ItemBoxState::QUIZ_COMPLETE ||
                      currentState == ItemBoxState::OPEN);
    if (!needsRfid) return QCResult();

    unsigned long now = millis();
    if (lastRfidAckMs == 0) {
      return QCResult(QCLevel::WARN, getId(), "RFID Runtime Ack",
                      "ACK within 5s", "No ACK yet",
                      "Check SPI wiring and PN532 power");
    }
    unsigned long idleMs = now - lastRfidAckMs;
    if (idleMs > 5000UL) {
      return QCResult(QCLevel::FAIL, getId(), "RFID Runtime Ack",
                      "<= 5000ms", String(idleMs) + "ms",
                      "RFID link lost, check cable/connectors/noise");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Static Pin Conflict Check
// ---------------------------------------------------------
class QCRule_PinConflict : public IQCRule {
public:
  String getId() const override { return "HW_PIN_01"; }
  String getName() const override { return "Pin Conflict Check"; }
  bool isFastCheck() const override { return true; }

  QCResult check() override {
    if (RELAY_PIN == VIBRATION_RANGE_PIN) {
      return QCResult(QCLevel::FAIL, getId(), "GPIO Assignment",
                      "Unique output pin per actuator",
                      "RELAY_PIN == VIBRATION_RANGE_PIN == " + String(RELAY_PIN),
                      "Split relay and vibration to different GPIOs");
    }
    if (PN532_SS1 == PN532_SS2) {
      return QCResult(QCLevel::FAIL, getId(), "PN532 SS Pins",
                      "Different SS pins", String(PN532_SS1),
                      "Assign unique SS pin per PN532 module");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: GPIO Capability Mismatch Check
// ---------------------------------------------------------
class QCRule_GpioCapability : public IQCRule {
public:
  String getId() const override { return "HW_GPIO_01"; }
  String getName() const override { return "GPIO Capability Check"; }
  bool isFastCheck() const override { return true; }

  QCResult check() override {
    // GPIO34~39 are input-only and do not support internal pull-up/down.
    if (buttonPin >= 34 && buttonPin <= 39) {
      return QCResult(QCLevel::FAIL, getId(), "buttonPin Capability",
                      "Pull-up capable GPIO or external resistor",
                      "GPIO" + String(buttonPin),
                      "Use external pull resistor or move button pin");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Encoder Value Range
// ---------------------------------------------------------
class QCRule_EncoderRange : public IQCRule {
public:
  String getId() const override { return "HW_ENC_01"; }
  String getName() const override { return "Encoder Value Range"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    extern volatile long encoderValue;
    long val = encoderValue;
    if (val < 0 || val > 380) {
      return QCResult(QCLevel::WARN, getId(), "Encoder Value",
                      "0~380", String(val),
                      "Encoder value out of range, check hardware");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: FSM vs Server State Consistency
// ---------------------------------------------------------
class QCRule_StateConsistency : public IQCRule {
public:
  String getId() const override { return "LOGIC_FSM_01"; }
  String getName() const override { return "FSM State Consistency"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    extern ItemBoxState currentState;
    if (my.size() == 0) return QCResult();

    if (my.containsKey("device_state")) {
      String dState = (const char *)my["device_state"];
      bool mismatch = false;
      String detail = "";

      if (dState == "open" && currentState != ItemBoxState::OPEN
          && currentState != ItemBoxState::USED
          && currentState != ItemBoxState::QUIZ_COMPLETE) {
        mismatch = true;
      }
      else if (dState == "used" && currentState != ItemBoxState::USED) {
        mismatch = true;
      }
      else if (dState == "activate" && currentState != ItemBoxState::ACTIVATE
               && currentState != ItemBoxState::PUZZLE
               && currentState != ItemBoxState::QUIZ_COMPLETE
               && currentState != ItemBoxState::OPEN) {
        mismatch = true;
      }

      if (mismatch) {
        detail = "Server:" + dState + " FSM:" + String(getStateName(currentState));
        return QCResult(QCLevel::WARN, getId(), "FSM vs Server",
                        "Consistent", detail,
                        "State mismatch between server and device");
      }
    }
    return QCResult();
  }
};

#endif // QC_RULES_H
