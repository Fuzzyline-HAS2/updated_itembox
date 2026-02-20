"""
ItemBox Pure-Python FSM Model
=============================
펌웨어 ItemBoxState.h + Wifi.ino + Game_system.ino + rfid.ino 로직을
순수 파이썬으로 1:1 모델링.

하드웨어 함수(BoxOpen, AllNeoOn 등)는 호출 기록만 남김.
has2wifi.Send()는 outbox 큐에 메시지 추가.
"""

from __future__ import annotations

import copy
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import Any


# ── ItemBoxState enum (ItemBoxState.h 12-state FSM) ─────────────────
class State(Enum):
    BOOT = auto()
    SETTING = auto()
    READY = auto()
    ACTIVATE = auto()
    PUZZLE = auto()
    QUIZ_COMPLETE = auto()
    OPEN = auto()
    USED = auto()
    CLOSE = auto()
    REPAIRED_ALL = auto()
    PLAYER_WIN = auto()
    PLAYER_LOSE = auto()


# ── Neopixel color constants ────────────────────────────────────────
WHITE, RED, YELLOW, GREEN, BLUE, PURPLE, BLACK = range(7)
COLOR_NAMES = {
    WHITE: "WHITE", RED: "RED", YELLOW: "YELLOW", GREEN: "GREEN",
    BLUE: "BLUE", PURPLE: "PURPLE", BLACK: "BLACK",
}


# ── Side-effect log entry ──────────────────────────────────────────
@dataclass
class HWCall:
    """하드웨어 함수 호출 기록"""
    func: str
    args: tuple = ()
    time: float = 0.0


@dataclass
class SendMsg:
    """has2wifi.Send() 호출 기록"""
    device: str
    key: str
    value: str
    time: float = 0.0


# ── Puzzle answer configuration (updated_itembox.h 기본값) ──────────
DEFAULT_ANSWERS = [13, 43, 21]        # modeValue[ANSWER]
DEFAULT_ANSWER_CNT = 3                # modeValue[RANGE][ANSWER_CNT]
DEFAULT_ANSWER_RANGE = 2              # modeValue[RANGE][ANSWER_RANGE]
DEFAULT_VIBRATION_RANGE = 5           # modeValue[RANGE][VIBRATION_RANGE]
DEFAULT_ENCODER_INIT = 165            # encoderValue 초기값


class ItemBoxModel:
    """
    아이템박스 펌웨어 FSM의 순수 파이썬 모델.

    Usage::

        model = ItemBoxModel()
        model.boot()
        model.on_server_message(game_state="setting")
        assert model.state == State.SETTING
    """

    def __init__(
        self,
        answers: list[int] | None = None,
        answer_cnt: int = DEFAULT_ANSWER_CNT,
        answer_range: int = DEFAULT_ANSWER_RANGE,
    ):
        # ── FSM ─────────────────────────────────────────────
        self.state: State = State.BOOT

        # ── Game variables ──────────────────────────────────
        self.encoder_value: int = DEFAULT_ENCODER_INIT
        self.answer_cnt: int = 0
        self.answers: list[int] = list(answers or DEFAULT_ANSWERS)
        self.max_answer_cnt: int = answer_cnt
        self.answer_range: int = answer_range
        self.self_open: bool = False
        self.used: bool = False

        # ── Mode pointers (문자열로 표현) ───────────────────
        self.current_mode: str = "WaitFunc"
        self.rfid_mode: str = "WaitFunc"

        # ── Server data (my / cur JSON) ─────────────────────
        self.my: dict[str, Any] = {}
        self.cur: dict[str, Any] = {}
        self.tag: dict[str, Any] = {}

        # ── Battery pack random distribution ────────────────
        self.battery_packs: list[dict] = [
            {"name": name, "val": 0}
            for name in ["HI1", "HI2", "OI1", "OI2", "BI1", "BI2",
                          "GI1", "GI2", "FI1", "FI2"]
        ]

        # ── Side-effect logs ────────────────────────────────
        self.hw_log: list[HWCall] = []
        self.outbox: list[SendMsg] = []
        self.fsm_log: list[tuple[State, State]] = []
        self.serial_log: list[str] = []

        # ── Timers (시뮬레이션) ─────────────────────────────
        self.sim_time: float = 0.0
        self.game_timer_active: bool = False
        self.game_timer_elapsed: float = 0.0
        self.game_timer_interval: float = 5.0  # 5초
        self.wifi_timer_active: bool = False
        self.blink_timer_active: bool = False

        # ── Encoder interrupt state ─────────────────────────
        self.encoder_interrupt_attached: bool = False

        # ── RFID init state ─────────────────────────────────
        self.rfid_init_complete: list[bool] = [False, False]
        self.rfid_init_attempted: bool = False
        self.last_rfid_ack_ms: float = 0.0

        # ── WiFi state ──────────────────────────────────────
        self.wifi_connected: bool = True
        self.wifi_rssi: int = -50

        # ── System state ────────────────────────────────────
        self.free_heap: int = 200000
        self.last_reset_reason: str = "ESP_RST_POWERON"

        # ── Pin configuration (Library_and_pin.h mirror) ────
        self.pin_config: dict = {
            "RELAY_PIN": 14,
            "VIBRATION_RANGE_PIN": 14,
            "PN532_SS1": 5,
            "PN532_SS2": 21,
            "buttonPin": 34,
        }

        # ── QC results ──────────────────────────────────────
        self.qc_results: list[dict] = []

    # ── Transition ──────────────────────────────────────────────
    def transition_to(self, new_state: State) -> bool:
        """ItemBoxState.h transitionTo() 재현"""
        if self.state == new_state:
            return False
        old = self.state
        self.fsm_log.append((old, new_state))
        self._serial(f"[FSM] {old.name} -> {new_state.name}")
        self.state = new_state
        return True

    # ── Hardware stubs (호출 기록만) ────────────────────────────
    def _hw(self, func: str, *args):
        self.hw_log.append(HWCall(func, args, self.sim_time))

    def _send(self, device: str, key: str, value: str):
        self.outbox.append(SendMsg(device, key, value, self.sim_time))

    def _serial(self, msg: str):
        self.serial_log.append(msg)

    def box_open(self):
        self._hw("BoxOpen")

    def box_close(self):
        self._hw("BoxClose")

    def all_neo_on(self, color: int):
        self._hw("AllNeoOn", color)

    def neo_blink(self, neo: int, color: int, cnt: int, blink_time: int):
        self._hw("NeoBlink", neo, color, cnt, blink_time)

    def light_color(self, strip_idx: int, color: int):
        self._hw("lightColor", strip_idx, color)

    def send_command(self, cmd: str):
        self._hw("sendCommand", cmd)

    def ledc_write(self, pin: int, value: int):
        self._hw("ledcWrite", pin, value)

    # ── Timer management ────────────────────────────────────────
    def _delete_game_timer(self):
        self.game_timer_active = False
        self.game_timer_elapsed = 0.0

    def _start_game_timer(self):
        self.game_timer_active = True
        self.game_timer_elapsed = 0.0

    def _delete_blink_timer(self):
        self.blink_timer_active = False

    def _start_blink_timer(self, neo: int, color: int):
        self.blink_timer_active = True
        self._hw("BlinkTimerStart", neo, color)

    def _delete_wifi_timer(self):
        self.wifi_timer_active = False

    def _start_wifi_timer(self):
        self.wifi_timer_active = True

    # ── Boot ────────────────────────────────────────────────────
    def boot(self):
        """setup() 시뮬레이션"""
        self.state = State.BOOT
        self.wifi_timer_active = True
        self._serial("QC System Started")

    # ── DataChanged (Wifi.ino) ──────────────────────────────────
    def on_server_message(
        self,
        game_state: str | None = None,
        device_state: str | None = None,
        manage_state: str | None = None,
        **extra,
    ):
        """
        Wifi.ino DataChanged() 1:1 재현.
        game_state / device_state 변경 시 핸들러 호출.
        """
        # Update 'my' data
        if game_state is not None:
            self.my["game_state"] = game_state
        if device_state is not None:
            self.my["device_state"] = device_state
        if manage_state is not None:
            self.my["manage_state"] = manage_state
        self.my.update(extra)

        # ── game_state changed ──────────────────────────────
        cur_gs = self.cur.get("game_state")
        my_gs = self.my.get("game_state")
        if my_gs is not None and my_gs != cur_gs:
            if my_gs == "setting":
                self._setting_func()
            elif my_gs == "ready":
                self._ready_func()
            elif my_gs == "activate":
                self._activate_func()

        # ── device_state changed ────────────────────────────
        cur_ds = self.cur.get("device_state")
        my_ds = self.my.get("device_state")
        if my_ds is not None and my_ds != cur_ds:
            if my_ds == "activate":
                self._activate_func()
            elif my_ds == "used":
                if not self.used:
                    self.transition_to(State.USED)
                    self.box_open()
                    self.all_neo_on(RED)
                    self.send_command("page pgItemTaken")
                    self.current_mode = "WaitFunc"
                    self.rfid_mode = "WaitFunc"
                    self.used = True
                    self._delete_blink_timer()
                    self._delete_game_timer()
            elif my_ds == "open":
                if not self.self_open:
                    self.transition_to(State.OPEN)
                    self._serial("PuzzleSolved")
                    self.all_neo_on(BLUE)
                    self.send_command("page pgItemOpen")
                    self.send_command("wOutTagged.en=1")
                    self.box_open()
                    self.light_color(2, YELLOW)  # INNER
                    self.current_mode = "RfidLoopInner"
                    self.rfid_mode = "ItemTook"
                    self._delete_blink_timer()
                    self._start_blink_timer(2, YELLOW)
                    self._delete_game_timer()
                    self.self_open = True
            elif my_ds == "close":
                self.transition_to(State.CLOSE)
                self.box_close()
            elif my_ds == "repaired_all":
                self.transition_to(State.REPAIRED_ALL)
                self.current_mode = "WaitFunc"
                self.rfid_mode = "WaitFunc"
                self.all_neo_on(BLUE)
                self.box_open()
                self.send_command("page pgEscapeOpen")
            elif my_ds == "player_win":
                self.transition_to(State.PLAYER_WIN)
                self.current_mode = "WaitFunc"
                self.rfid_mode = "WaitFunc"
                self.all_neo_on(BLUE)
                self.box_open()
                self.send_command("page pgPlayerWin")
            elif my_ds == "player_lose":
                self.transition_to(State.PLAYER_LOSE)
                self.current_mode = "WaitFunc"
                self.rfid_mode = "WaitFunc"
                self.all_neo_on(RED)
                self.box_open()
                self.send_command("page pgPlayerLose")

        # ── cur = my ────────────────────────────────────────
        self.cur = copy.deepcopy(self.my)

    # ── Handler functions (Wifi.ino) ────────────────────────────
    def _setting_func(self):
        """SettingFunc()"""
        self.transition_to(State.SETTING)
        self.send_command("page pgWait")
        self._serial("SETTING")
        self.all_neo_on(WHITE)
        self.box_open()
        self.encoder_value = DEFAULT_ENCODER_INIT
        self.answer_cnt = 0
        self.current_mode = "WaitFunc"
        self.rfid_mode = "WaitFunc"
        self.self_open = False
        self.used = False
        self._delete_blink_timer()
        self._delete_game_timer()
        self.ledc_write(14, 0)  # VIBRATION_RANGE_PIN

    def _activate_func(self):
        """ActivateFunc()"""
        self.transition_to(State.ACTIVATE)
        self.send_command("page pgWait")
        self.encoder_value = DEFAULT_ENCODER_INIT
        self.answer_cnt = 0
        self._serial("ACTIVATE")
        self.all_neo_on(YELLOW)
        self.box_close()
        self.current_mode = "RfidLoopOutter"
        self.rfid_mode = "StartPuzzle"
        self.self_open = False
        self.used = False
        self._delete_blink_timer()
        self._delete_game_timer()
        self.ledc_write(14, 0)

    def _ready_func(self):
        """ReadyFunc() — 배터리팩 랜덤 분배 포함"""
        self.transition_to(State.READY)

        # 배터리팩 초기화
        for bp in self.battery_packs:
            bp["val"] = 0

        # 랜덤 분배 (각 팩 최대 3개, 총 10개)
        import random
        cycle = 0
        while cycle < 10:
            idx = random.randint(0, 9)
            if self.battery_packs[idx]["val"] < 3:
                self.battery_packs[idx]["val"] += 1
                cycle += 1

        # 0이 아닌 팩만 서버로 전송
        for bp in self.battery_packs:
            if bp["val"] != 0:
                self._send(bp["name"], "battery_pack", str(bp["val"]))

        self.send_command("page pgWait")
        self._serial("READY")
        self.all_neo_on(RED)
        self.box_close()
        self.current_mode = "WaitFunc"
        self.rfid_mode = "WaitFunc"
        self.self_open = False
        self.used = False
        self._delete_blink_timer()
        self._delete_game_timer()
        self.ledc_write(14, 0)

    # ── RFID tag event ──────────────────────────────────────────
    def rfid_tag(self, role: str, tag_data: str = "G1P1",
                 battery_pack: int = 0, max_battery_pack: int = 10,
                 device_name: str = "player1", exp_pack: str = "0"):
        """
        CheckingPlayers() 시뮬레이션.
        role: "player" | "tagger" | "ghost"
        """
        # RFID ACK 갱신 (heartbeat)
        self.last_rfid_ack_ms = self.sim_time

        # MMMM 태그 → 리셋
        if tag_data == "MMMM":
            self._serial("ESP.restart()")
            self.boot()
            return

        self.tag = {
            "role": role,
            "device_name": device_name,
            "battery_pack": battery_pack,
            "max_battery_pack": max_battery_pack,
            "exp_pack": exp_pack,
        }

        self._serial(f"tag_user_data : {tag_data}")

        if role == "player":
            self._serial("Player Tagged")
            self._call_rfid_mode()
        elif role == "tagger":
            self._serial("Tagger Tagged")
        elif role == "ghost":
            self._serial("Ghost Tagged")
        else:
            self._serial("Wrong TAG")

    def _call_rfid_mode(self):
        """ptrRfidMode() 호출"""
        if self.rfid_mode == "StartPuzzle":
            self._start_puzzle()
        elif self.rfid_mode == "PuzzleSolved":
            self._puzzle_solved()
        elif self.rfid_mode == "ItemTook":
            self._item_took()
        elif self.rfid_mode == "WaitFunc":
            pass  # no-op

    # ── StartPuzzle (rfid.ino) ──────────────────────────────────
    def _start_puzzle(self):
        self.transition_to(State.PUZZLE)
        self._serial("StartPuzzle")
        self._delete_wifi_timer()
        self._delete_game_timer()
        self._start_game_timer()
        self.answer_cnt = 0
        self.current_mode = "Puzzle"
        self.all_neo_on(BLUE)
        self.encoder_interrupt_attached = True

    # ── Puzzle - submit encoder value ───────────────────────────
    def submit_encoder(self, button_pressed: bool = True) -> str:
        """
        Game_system.ino Puzzle() 의 엔코더 입력 시뮬레이션.
        Returns: "correct", "wrong", or "quiz_complete"
        """
        if self.current_mode != "Puzzle":
            return "not_in_puzzle"

        if not button_pressed:
            return "no_press"

        current_answer = self.answers[self.answer_cnt]
        difference = abs(current_answer - (self.encoder_value // 4)) // self.answer_range

        if difference == 0:
            self._serial("Correct Answer")
            self.neo_blink(2, GREEN, 5, 250)  # ENCODER
            self.answer_cnt += 1

            if self.answer_cnt >= self.max_answer_cnt:
                self._serial("QUIZ SUCCEED")
                self.send_command("wQuizSolved.en=1")
                self.ledc_write(14, 0)
                self.answer_cnt = 0
                self.encoder_interrupt_attached = False
                self._delete_blink_timer()
                self._start_blink_timer(0, YELLOW)  # PN532
                self._delete_wifi_timer()
                self._start_wifi_timer()
                self.game_timer_elapsed = 0.0
                self.current_mode = "RfidLoopOutter"
                self.rfid_mode = "PuzzleSolved"
                self.transition_to(State.QUIZ_COMPLETE)
                return "quiz_complete"
            return "correct"
        else:
            self._serial("Wrong Answer")
            self.neo_blink(2, RED, 5, 250)
            return "wrong"

    # ── PuzzleSolved (rfid.ino) ─────────────────────────────────
    def _puzzle_solved(self):
        self.transition_to(State.OPEN)
        self.self_open = True
        device_name = self.my.get("device_name", "itembox")
        self._send(device_name, "device_state", "open")
        self._serial("PuzzleSolved")
        self.all_neo_on(BLUE)
        self.send_command("page pgItemOpen")
        self.send_command("wOutTagged.en=1")
        self.box_open()
        self._delete_blink_timer()
        self._start_blink_timer(2, YELLOW)  # INNER
        self._delete_game_timer()
        self.current_mode = "RfidLoopInner"
        self.rfid_mode = "ItemTook"

    # ── ItemTook (rfid.ino) ─────────────────────────────────────
    def _item_took(self):
        self._serial("ItemTook")
        tag_bp = self.tag.get("battery_pack", 0)
        my_bp = int(self.my.get("battery_pack", 0))
        max_bp = self.tag.get("max_battery_pack", 10)

        if (tag_bp + my_bp) <= max_bp:
            self.transition_to(State.USED)
            self.send_command("page pgItemTaken")
            self.all_neo_on(RED)
            device_name = self.my.get("device_name", "itembox")
            tag_device = self.tag.get("device_name", "player1")
            self._send(device_name, "device_state", "used")
            self._send(tag_device, "battery_pack",
                       f"+{self.my.get('battery_pack', '0')}")
            self._send(tag_device, "exp",
                       f"+{self.my.get('exp_pack', '0')}")
            self._send(device_name, "battery_pack",
                       f"-{self.my.get('battery_pack', '0')}")
            self._send(device_name, "exp_pack",
                       f"-{self.my.get('exp_pack', '0')}")
            self._delete_blink_timer()
            self.used = True
            self.current_mode = "WaitFunc"
            self.rfid_mode = "WaitFunc"
        else:
            self._serial("NOT ENOUGH IOT BatteryPack")
            self.send_command("page pgItemTakeFail")
            self.neo_blink(2, RED, 4, 250)
            self._delete_blink_timer()
            self._start_blink_timer(2, YELLOW)

    # ── Game timer timeout (timer.ino 시뮬) ─────────────────────
    def advance_time(self, dt: float):
        """시뮬레이션 시간 전진"""
        self.sim_time += dt
        if self.game_timer_active:
            self.game_timer_elapsed += dt
            if self.game_timer_elapsed >= self.game_timer_interval:
                self._on_game_timer_tick()

    def _on_game_timer_tick(self):
        """GameTimerFunc() — 퍼즐 타임아웃 시 activate로 복귀"""
        self.game_timer_elapsed = 0.0
        if self.current_mode == "Puzzle":
            # 타임아웃 → activate로 복귀
            self._activate_func()

    # ── Reset (ESP.restart() 시뮬) ─────────────────────────────
    def reset(self):
        """ESP.restart() 시뮬레이션 — 모든 상태 초기화 후 boot"""
        self.__init__(
            answers=list(self.answers),
            answer_cnt=self.max_answer_cnt,
            answer_range=self.answer_range,
        )
        self.boot()

    # ── QC Rules (파이썬 재구현) ────────────────────────────────
    def run_qc_check(self) -> list[dict]:
        """QC_Rules.h의 11개 룰을 파이썬으로 체크"""
        results = []

        # ── 1. NET_WIFI_00: WiFi Connection (FAST) ──────────
        if not self.wifi_connected:
            results.append({"id": "NET_WIFI_00", "level": "FAIL",
                            "what": "WiFi Status", "value": "Disconnected"})

        # ── 2. NET_WIFI_01: WiFi Signal (SLOW) ──────────────
        if self.wifi_connected:
            if self.wifi_rssi < -85:
                results.append({"id": "NET_WIFI_01", "level": "FAIL",
                                "what": "WiFi RSSI", "value": self.wifi_rssi})
            elif self.wifi_rssi < -75:
                results.append({"id": "NET_WIFI_01", "level": "WARN",
                                "what": "WiFi RSSI", "value": self.wifi_rssi})

        # ── 3. SYS_MEM_01: Heap Memory (SLOW) ──────────────
        if self.free_heap < 10000:
            results.append({"id": "SYS_MEM_01", "level": "FAIL",
                            "what": "Free Heap", "value": self.free_heap})
        elif self.free_heap < 20000:
            results.append({"id": "SYS_MEM_01", "level": "WARN",
                            "what": "Free Heap", "value": self.free_heap})

        # ── 4. SYS_RST_01: Reset Reason (SLOW, 1회 보고) ───
        if self.last_reset_reason == "ESP_RST_BROWNOUT":
            results.append({"id": "SYS_RST_01", "level": "FAIL",
                            "what": "Last Reset Reason",
                            "value": self.last_reset_reason})
        elif self.last_reset_reason in ("ESP_RST_WDT", "ESP_RST_INT_WDT",
                                         "ESP_RST_TASK_WDT"):
            results.append({"id": "SYS_RST_01", "level": "WARN",
                            "what": "Last Reset Reason",
                            "value": self.last_reset_reason})
        elif self.last_reset_reason == "ESP_RST_PANIC":
            results.append({"id": "SYS_RST_01", "level": "WARN",
                            "what": "Last Reset Reason",
                            "value": self.last_reset_reason})

        # ── 5. HW_PIN_01: Pin Conflict (FAST) ──────────────
        pc = self.pin_config
        if pc["RELAY_PIN"] == pc["VIBRATION_RANGE_PIN"]:
            results.append({"id": "HW_PIN_01", "level": "FAIL",
                            "what": "GPIO Assignment",
                            "value": f"RELAY_PIN == VIBRATION_RANGE_PIN == {pc['RELAY_PIN']}"})
        if pc["PN532_SS1"] == pc["PN532_SS2"]:
            results.append({"id": "HW_PIN_01", "level": "FAIL",
                            "what": "PN532 SS Pins",
                            "value": str(pc["PN532_SS1"])})

        # ── 6. HW_GPIO_01: GPIO Capability (FAST) ──────────
        btn = pc["buttonPin"]
        if 34 <= btn <= 39:
            results.append({"id": "HW_GPIO_01", "level": "FAIL",
                            "what": "buttonPin Capability",
                            "value": f"GPIO{btn}"})

        # ── 7. LOGIC_STATE_01: Game State Validity (SLOW) ──
        valid_gs = {"setting", "activate", "ready", "", None}
        gs = self.my.get("game_state")
        if gs is not None and gs not in valid_gs:
            results.append({"id": "LOGIC_STATE_01", "level": "WARN",
                            "what": "game_state", "value": gs})

        valid_ds = {"setting", "activate", "ready", "open", "used",
                    "close", "repaired_all", "player_win", "player_lose",
                    "", None}
        ds = self.my.get("device_state")
        if ds is not None and ds not in valid_ds:
            results.append({"id": "LOGIC_STATE_01", "level": "WARN",
                            "what": "device_state", "value": ds})

        ms = self.my.get("manage_state")
        valid_ms = {"mo", "mu", "", None}
        if ms is not None and ms not in valid_ms:
            results.append({"id": "LOGIC_STATE_01", "level": "WARN",
                            "what": "manage_state", "value": ms})

        # ── 8. HW_RFID_01: RFID Init (SLOW) ────────────────
        if self.rfid_init_attempted and not all(self.rfid_init_complete):
            results.append({"id": "HW_RFID_01", "level": "FAIL",
                            "what": "RFID Init",
                            "value": self.rfid_init_complete})

        # ── 9. HW_RFID_02: RFID Heartbeat (SLOW) ──────────
        if self.rfid_init_attempted:
            needs_rfid = self.state in (
                State.ACTIVATE, State.QUIZ_COMPLETE, State.OPEN)
            if needs_rfid:
                if self.last_rfid_ack_ms == 0:
                    results.append({"id": "HW_RFID_02", "level": "WARN",
                                    "what": "RFID Runtime Ack",
                                    "value": "No ACK yet"})
                else:
                    idle_ms = (self.sim_time - self.last_rfid_ack_ms) * 1000
                    if idle_ms > 5000:
                        results.append({"id": "HW_RFID_02", "level": "FAIL",
                                        "what": "RFID Runtime Ack",
                                        "value": f"{idle_ms:.0f}ms"})

        # ── 10. HW_ENC_01: Encoder Range (SLOW) ────────────
        if self.encoder_value < 0 or self.encoder_value > 380:
            results.append({"id": "HW_ENC_01", "level": "WARN",
                            "what": "Encoder Value",
                            "value": self.encoder_value})

        # ── 11. LOGIC_FSM_01: FSM vs Server (SLOW) ─────────
        ds = self.my.get("device_state")
        if ds:
            mismatch = False
            if ds == "open" and self.state not in (
                    State.OPEN, State.USED, State.QUIZ_COMPLETE):
                mismatch = True
            elif ds == "used" and self.state != State.USED:
                mismatch = True
            elif ds == "activate" and self.state not in (
                    State.ACTIVATE, State.PUZZLE,
                    State.QUIZ_COMPLETE, State.OPEN):
                mismatch = True
            if mismatch:
                results.append({
                    "id": "LOGIC_FSM_01", "level": "WARN",
                    "what": "FSM vs Server",
                    "value": f"Server:{ds} FSM:{self.state.name}",
                })

        self.qc_results = results
        return results

    # ── Helpers ─────────────────────────────────────────────────
    def get_hw_calls(self, func_name: str) -> list[HWCall]:
        """특정 하드웨어 함수 호출 기록 필터"""
        return [h for h in self.hw_log if h.func == func_name]

    def get_sends(self, key: str | None = None) -> list[SendMsg]:
        """outbox에서 특정 key의 메시지 필터"""
        if key is None:
            return list(self.outbox)
        return [m for m in self.outbox if m.key == key]

    def last_neo_color(self) -> int | None:
        """마지막 AllNeoOn 호출의 색상"""
        calls = self.get_hw_calls("AllNeoOn")
        return calls[-1].args[0] if calls else None
