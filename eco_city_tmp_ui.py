#!/usr/bin/env python3

# Eco City Game – Python TUI temporary UI for testing the C++ backend
# Connects to the C++ backend on TCP port 54321
#
# Protocol: 4-byte big-endian length header + 1-byte message-type tag + protobuf body
#   0x01 = GameState   (regular per-tick update)
#   0x02 = GameOver    (final message from engine before it closes the connection)
#
# On connect the UI sends a UIAction with game_id (field 3) set, so the
# backend knows which save file to load.
#
# Protobuf is hand-encoded/decoded here so there is NO external dependency.
# Only Python stdlib is used (socket, threading, curses, struct).

# Controls
# A – Accept current petition
# R – Reject current petition
# S – Save game
# Q – Quit UI  (backend keeps running)

import curses
import socket
import struct
import threading
import time
from dataclasses import dataclass, field
from typing import Optional

WIRE_VARINT   = 0
WIRE_LEN      = 2

# 1-byte message-type prefix added to every framed packet (new protocol)
MSG_GAME_STATE = 0x01
MSG_GAME_OVER  = 0x02


def _to_int32(val: int) -> int:
    if val >= (1 << 31):
        val -= (1 << 32)
    return val


def _as_int32(val: int) -> int:
    val &= 0xFFFFFFFF
    return val - 0x100000000 if val & 0x80000000 else val


def _encode_varint(value: int) -> bytes:
    bits = []
    value &= 0xFFFF_FFFF_FFFF_FFFF  # treat as unsigned 64-bit
    while value > 0x7F:
        bits.append((value & 0x7F) | 0x80)
        value >>= 7
    bits.append(value)
    return bytes(bits)


def _encode_field_varint(field_num: int, value: int) -> bytes:
    tag = (field_num << 3) | WIRE_VARINT
    return _encode_varint(tag) + _encode_varint(value)


def _encode_field_bool(field_num: int, value: bool) -> bytes:
    return _encode_field_varint(field_num, 1 if value else 0)


def _encode_field_bytes(field_num: int, data: bytes) -> bytes:
    tag = (field_num << 3) | WIRE_LEN
    return _encode_varint(tag) + _encode_varint(len(data)) + data


def _encode_field_string(field_num: int, text: str) -> bytes:
    """Encode a UTF-8 string as a protobuf length-delimited field."""
    encoded = text.encode("utf-8")
    return _encode_field_bytes(field_num, encoded)


def _decode_varint(data: bytes, pos: int):
    result = 0
    shift = 0
    while True:
        b = data[pos]; pos += 1
        result |= (b & 0x7F) << shift
        if not (b & 0x80):
            break
        shift += 7
    return result, pos


def _decode_fields(data: bytes):
    # Yield (field_number, wire_type, value_bytes_or_int) tuples
    pos = 0
    n = len(data)
    while pos < n:
        tag, pos = _decode_varint(data, pos)
        field_num = tag >> 3
        wire_type = tag & 0x7
        if wire_type == WIRE_VARINT:
            val, pos = _decode_varint(data, pos)
            yield field_num, wire_type, val
        elif wire_type == WIRE_LEN:
            length, pos = _decode_varint(data, pos)
            payload = data[pos:pos + length]; pos += length
            yield field_num, wire_type, payload
        else:
            break  # unknown wire type – stop parsing

RESOURCE_NAMES = {
    0: "Unspecified",
    1: "Water (kL)",
    2: "Energy (MWh)",
    3: "Money (100k £)",
    4: "Population",
    5: "CO₂ (5t)",
}

BUILDING_NAMES = {
    0:  "Unspecified",
    1:  "Power Plant",
    2:  "Water Treatment Plant",
    3:  "Solar Panel Farm",
    4:  "Solar Panel Rooftops",
    5:  "Public Transport Upgrade",
    6:  "Wind Turbine Farm",
    7:  "Hydroelectric Plant",
    8:  "Urban Greening",
    9:  "Water Saving Infrastructure",
    10: "Industrial Zone",
    11: "Airport Expansion",
    12: "Road Improvement",
}

@dataclass
class ResourceEffect:
    resource_type: int = 0
    delta_value: int = 0

@dataclass
class Building:
    type: int = 0
    build_cost: int = 0
    ticks_to_complete: int = 0
    effects: list = field(default_factory=list)

@dataclass
class Petition:
    id: int = 0
    building: Optional[Building] = None

@dataclass
class GameState:
    building_counts: dict = field(default_factory=dict)   # {BuildingType -> count}
    resources: dict = field(default_factory=dict)
    current_petition: Optional[Petition] = None


# Reason strings for game-over screen.
# Keys are the GameOverReason enum values from the proto (ints), with a
# string fallback key "disconnected" for unexpected disconnects.
GAME_OVER_REASONS = {
    1: "A critical resource has run out.",
    2: "CO₂ emissions have exceeded the safe limit!",
    "disconnected": "The backend disconnected unexpectedly.",
}

MAX_CO2 = 100_000_000  # must match C++ GameService.hpp


@dataclass
class GameOver:
    reason: int = 0                  # GameOverReason enum value (1=resource, 2=co2)
    final_resources: dict = field(default_factory=dict)


def parse_game_over(data: bytes) -> GameOver:
    """Parse a GameOver protobuf message (field 1 = reason, field 2 = final_resources map)."""
    go = GameOver()
    for fn, wt, val in _decode_fields(data):
        if fn == 1 and wt == WIRE_VARINT:
            go.reason = val
        elif fn == 2 and wt == WIRE_LEN:
            res_key = 0
            res_val = 0
            for mf, _, mv in _decode_fields(val):
                if mf == 1:
                    res_key = mv
                elif mf == 2:
                    res_val = mv
            go.final_resources[_as_int32(res_key)] = _as_int32(res_val)
    return go


def _parse_resource_effect(data: bytes) -> ResourceEffect:
    e = ResourceEffect()
    for fn, wt, val in _decode_fields(data):
        if fn == 1 and wt == WIRE_VARINT:
            e.resource_type = val
        elif fn == 2 and wt == WIRE_VARINT:
            # proto int32 – handle sign
            e.delta_value = _as_int32(val)
    return e


def _parse_building(data: bytes) -> Building:
    b = Building()
    for fn, wt, val in _decode_fields(data):
        if fn == 1 and wt == WIRE_VARINT:
            b.type = val
        elif fn == 2 and wt == WIRE_VARINT:
            b.build_cost = val
        elif fn == 3 and wt == WIRE_VARINT:
            b.ticks_to_complete = val
        elif fn == 4 and wt == WIRE_LEN:
            b.effects.append(_parse_resource_effect(val))
    return b


def _parse_petition(data: bytes) -> Petition:
    p = Petition()
    for fn, wt, val in _decode_fields(data):
        if fn == 1 and wt == WIRE_VARINT:
            p.id = val
        elif fn == 2 and wt == WIRE_LEN:
            p.building = _parse_building(val)
    return p


def parse_game_state(data: bytes) -> GameState:
    gs = GameState()

    for fn, wt, val in _decode_fields(data):

        # building_counts map
        if fn == 1 and wt == WIRE_LEN:
            # map<int32, int32> entry: fields 1 (key) and 2 (value)
            map_key = 0
            map_val = 0
            for mf, _, mv in _decode_fields(val):
                if mf == 1:
                    map_key = mv
                elif mf == 2:
                    map_val = mv
            gs.building_counts[_as_int32(map_key)] = _as_int32(map_val)

        # resources map
        elif fn == 5 and wt == WIRE_LEN:
            # map<int32, int32> resources
            res_key = 0
            res_val = 0
            for mf, _, mv in _decode_fields(val):
                if mf == 1:
                    res_key = mv
                elif mf == 2:
                    res_val = mv
            gs.resources[_as_int32(res_key)] = _as_int32(res_val)

        # petition
        elif fn == 4 and wt == WIRE_LEN:
            gs.current_petition = _parse_petition(val)

    return gs


def encode_ui_action(accept: Optional[bool] = None, save: bool = False,
                     game_id: Optional[str] = None) -> bytes:
    # UIAction:
    # field 1: PetitionResponse { field 1: responded (bool), field 2: accepted (bool) }
    # field 2: save_game (bool)
    # field 3: game_id (string) – sent once on connect

    body = b""
    if accept is not None:
        pr = _encode_field_bool(1, True) + _encode_field_bool(2, accept)
        body += _encode_field_bytes(1, pr)
    if save:
        body += _encode_field_bool(2, True)
    if game_id:
        body += _encode_field_string(3, game_id)
    return body


HOST = "127.0.0.1"
PORT = 54321
RECONNECT_DELAY = 2.0


class GameConnection:
    def __init__(self, game_id: str = "local_game"):
        self._sock: Optional[socket.socket] = None
        self._lock = threading.Lock()
        self.connected = False
        self.last_state: Optional[GameState] = None
        self.game_over_info: Optional[GameOver] = None   # set when engine sends GameOver
        self.status_msg = "Connecting…"
        self._running = True
        self.game_over = False
        self.game_over_reason = "disconnected"
        self._game_id = game_id
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()


    def _run(self):
        while self._running:
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.settimeout(5.0)
                s.connect((HOST, PORT))
                s.settimeout(None)
                with self._lock:
                    self._sock = s
                    self.connected = True
                    self.status_msg = f"Connected to {HOST}:{PORT}"
                # Announce our game ID to the backend immediately after connecting
                self._send_raw(encode_ui_action(game_id=self._game_id))
                self._recv_loop(s)
            except (ConnectionRefusedError, OSError) as e:
                with self._lock:
                    self.connected = False
                    self.status_msg = f"Cannot connect ({e}) – retrying in {RECONNECT_DELAY}s…"
            finally:
                with self._lock:
                    if self._sock:
                        try: self._sock.close()
                        except: pass
                    self._sock = None
                    self.connected = False
            if self._running:
                time.sleep(RECONNECT_DELAY)


    def _recv_loop(self, sock: socket.socket):
        try:
            while self._running:
                # Read framed packet: [4-byte length][payload]
                # For GameOver frames the payload starts with a 0x02 tag byte (new protocol).
                # GameState frames are raw protobuf with NO tag byte (existing C++ behaviour).
                header = self._recvall(sock, 4)
                if not header:
                    # Backend closed without sending a GameOver – unexpected disconnect
                    with self._lock:
                        self.status_msg = "Disconnected – waiting for backend…"
                        if self.last_state is not None and not self.game_over:
                            self.game_over = True
                            self.game_over_reason = "disconnected"
                    return
                length = struct.unpack(">I", header)[0]
                if length == 0 or length > 1_048_576:
                    return
                frame = self._recvall(sock, length)
                if not frame:
                    return

                # Detect message type by peeking at the first byte.
                # 0x02 = GameOver (new, prefixed protocol).
                # Anything else = raw GameState protobuf (existing C++ sendGameState).
                # Note: a valid GameState proto's first byte is always a protobuf tag
                # varint, which for field 1 (wire 2) = 0x0A, field 4 = 0x22,
                # field 5 = 0x2A — never 0x02 — so the check is unambiguous.
                try:
                    if frame[0] == MSG_GAME_OVER:
                        go = parse_game_over(frame[1:])
                        with self._lock:
                            self.game_over_info = go
                            self.game_over = True
                            self.game_over_reason = go.reason
                            if self.last_state is not None and go.final_resources:
                                self.last_state.resources = go.final_resources
                            self.status_msg = "Game over."
                        return
                    else:
                        # Raw GameState — the whole frame is the protobuf payload
                        gs = parse_game_state(frame)
                        with self._lock:
                            self.last_state = gs
                            self.status_msg = f"Connected  |  last update: {time.strftime('%H:%M:%S')}"
                except Exception as e:
                    with self._lock:
                        self.status_msg = f"Parse error: {e}"
        except OSError:
            pass


    @staticmethod
    def _recvall(sock: socket.socket, n: int) -> Optional[bytes]:
        buf = b""
        while len(buf) < n:
            chunk = sock.recv(n - len(buf))
            if not chunk:
                return None
            buf += chunk
        return buf


    def _send_raw(self, payload: bytes) -> bool:
        """Send a length-framed payload (no message-type prefix — UI→backend messages don't use it)."""
        header = struct.pack(">I", len(payload))
        with self._lock:
            sock = self._sock
        if sock:
            try:
                sock.sendall(header + payload)
                return True
            except OSError:
                pass
        return False

    def send_action(self, accept: Optional[bool] = None, save: bool = False):
        return self._send_raw(encode_ui_action(accept=accept, save=save))


    def stop(self):
        self._running = False
        with self._lock:
            if self._sock:
                try: self._sock.close()
                except: pass

COLORS = {
    "title":    1,
    "header":   2,
    "good":     3,
    "warn":     4,
    "bad":      5,
    "neutral":  6,
    "dim":      7,
    "accept":   8,
    "reject":   9,
    "save":     10,
    "box":      11,
}

resource_cache = {}

def init_colors():
    curses.start_color()
    curses.use_default_colors()
    curses.init_pair(COLORS["title"],   curses.COLOR_CYAN,    -1)
    curses.init_pair(COLORS["header"],  curses.COLOR_WHITE,   -1)
    curses.init_pair(COLORS["good"],    curses.COLOR_GREEN,   -1)
    curses.init_pair(COLORS["warn"],    curses.COLOR_YELLOW,  -1)
    curses.init_pair(COLORS["bad"],     curses.COLOR_RED,     -1)
    curses.init_pair(COLORS["neutral"], curses.COLOR_WHITE,   -1)
    curses.init_pair(COLORS["dim"],     curses.COLOR_WHITE,   -1)
    curses.init_pair(COLORS["accept"],  curses.COLOR_BLACK,   curses.COLOR_GREEN)
    curses.init_pair(COLORS["reject"],  curses.COLOR_BLACK,   curses.COLOR_RED)
    curses.init_pair(COLORS["save"],    curses.COLOR_BLACK,   curses.COLOR_CYAN)
    curses.init_pair(COLORS["box"],     curses.COLOR_CYAN,    -1)


def cp(name: str):
    return curses.color_pair(COLORS[name])


def safe_addstr(win, y, x, text, attr=0):
    max_y, max_x = win.getmaxyx()
    if y < 0 or y >= max_y or x < 0 or x >= max_x:
        return
    available = max_x - x
    if available <= 0:
        return
    try:
        win.addstr(y, x, text[:available], attr)
    except curses.error:
        pass


def draw_box(win, y, x, h, w, title=""):
    max_y, max_x = win.getmaxyx()
    attr = cp("box")
    # Top
    safe_addstr(win, y, x, "┌" + "─" * (w - 2) + "┐", attr)
    if title:
        safe_addstr(win, y, x + 2, f" {title} ", attr | curses.A_BOLD)
    # Sides
    for row in range(1, h - 1):
        if y + row < max_y:
            safe_addstr(win, y + row, x, "│", attr)
            safe_addstr(win, y + row, x + w - 1, "│", attr)
    # Bottom
    if y + h - 1 < max_y:
        safe_addstr(win, y + h - 1, x, "└" + "─" * (w - 2) + "┘", attr)


def resource_color(rtype: int, amount: int) -> str:
    if rtype == 5:  # CO2 – high is bad
        if amount > 800: return "bad"
        if amount > 500: return "warn"
        return "good"
    # others – low is bad
    if amount < 50:  return "bad"
    if amount < 200: return "warn"
    return "good"


def render_game_over(stdscr, conn: GameConnection):
    """
    Full-screen game-over overlay.
    Keys:  N = new game (restart backend, same UI loop)
           Q = quit UI entirely
    Returns True if the user wants to quit, False if they want a new game.
    """
    stdscr.nodelay(False)  # block on input while the overlay is shown

    with conn._lock:
        reason_key = conn.game_over_reason   # int (proto enum) or "disconnected"
        go_info    = conn.game_over_info
        state      = conn.last_state

    reason_text = GAME_OVER_REASONS.get(reason_key, "The game has ended.")

    # Build a short final-stats summary: prefer the GameOver final_resources snapshot
    # (accurate at the exact moment the engine stopped), fall back to last GameState.
    resources_to_show = {}
    if go_info and go_info.final_resources:
        resources_to_show = go_info.final_resources
    elif state:
        resources_to_show = state.resources

    stats_lines = []
    for rtype in sorted(resources_to_show):
        rname  = RESOURCE_NAMES.get(rtype, f"Resource {rtype}")
        amount = resources_to_show[rtype]
        stats_lines.append(f"  {rname:<22} {amount:>12,}")

    while True:
        stdscr.erase()
        h, w = stdscr.getmaxyx()

        # Darken the whole screen
        for row in range(h):
            safe_addstr(stdscr, row, 0, " " * w, cp("dim") | curses.A_REVERSE)

        # Centre the overlay box
        box_h = max(14, len(stats_lines) + 10)
        box_w = min(60, w - 4)
        box_y = max(0, (h - box_h) // 2)
        box_x = max(0, (w - box_w) // 2)

        draw_box(stdscr, box_y, box_x, box_h, box_w, " GAME OVER ")

        # Title
        label = "★  GAME OVER  ★"
        safe_addstr(stdscr, box_y + 1, box_x + (box_w - len(label)) // 2,
                    label, cp("bad") | curses.A_BOLD)

        # Reason
        safe_addstr(stdscr, box_y + 2, box_x + 2,
                    reason_text[:box_w - 4], cp("warn"))

        # Stats header
        if stats_lines:
            safe_addstr(stdscr, box_y + 4, box_x + 2,
                        "Final Resources:", cp("header") | curses.A_UNDERLINE)
            for i, line in enumerate(stats_lines):
                if box_y + 5 + i >= box_y + box_h - 4:
                    break
                safe_addstr(stdscr, box_y + 5 + i, box_x + 2,
                            line[:box_w - 4], cp("neutral"))

        # Action prompt
        prompt_y = box_y + box_h - 3
        safe_addstr(stdscr, prompt_y, box_x + 2,
                    "[ N ]", cp("accept") | curses.A_BOLD)
        safe_addstr(stdscr, prompt_y, box_x + 9,
                    " New game  (restart the backend)", cp("good"))
        safe_addstr(stdscr, prompt_y + 1, box_x + 2,
                    "[ Q ]", cp("reject") | curses.A_BOLD)
        safe_addstr(stdscr, prompt_y + 1, box_x + 9,
                    " Quit", cp("bad"))

        stdscr.refresh()

        key = stdscr.getch()
        if key == curses.ERR:
            continue
        ch = chr(key).lower() if 0 < key < 256 else ""
        if ch == "q":
            return True   # quit
        if ch == "n":
            return False  # new game


def render_new_game_prompt(stdscr, current_game_id: str = "local_game") -> str:
    """
    Shown after the user pressed N on the game-over screen, or on first launch.
    Lets the player type a game ID (default: current_game_id).
    Returns the chosen game ID string.
    """
    stdscr.nodelay(False)
    curses.echo()
    curses.curs_set(1)

    game_id = current_game_id

    while True:
        stdscr.erase()
        h, w = stdscr.getmaxyx()

        for row in range(h):
            safe_addstr(stdscr, row, 0, " " * w, cp("dim") | curses.A_REVERSE)

        box_h = 14
        box_w = min(68, w - 4)
        box_y = max(0, (h - box_h) // 2)
        box_x = max(0, (w - box_w) // 2)

        draw_box(stdscr, box_y, box_x, box_h, box_w, " NEW GAME ")

        lines = [
            "Enter a game ID to load or create a save.",
            "Leave blank to use the default (local_game).",
            "",
            "  Each game ID maps to a separate MongoDB document,",
            "  so you can keep multiple independent saves.",
            "",
            "After confirming, restart the backend with:",
            "  eco_city_engine --game-id <your_id>",
            "",
            "Press ENTER to confirm.",
        ]
        for i, line in enumerate(lines):
            if box_y + 1 + i >= box_y + box_h - 3:
                break
            safe_addstr(stdscr, box_y + 1 + i, box_x + 2,
                        line[:box_w - 4], cp("neutral"))

        # Input field
        field_y = box_y + box_h - 3
        label = "Game ID: "
        safe_addstr(stdscr, field_y, box_x + 2, label, cp("header") | curses.A_BOLD)
        field_x = box_x + 2 + len(label)
        field_w = box_w - 2 - len(label) - 2

        # Draw input area
        safe_addstr(stdscr, field_y, field_x, " " * field_w, cp("neutral") | curses.A_REVERSE)
        safe_addstr(stdscr, field_y, field_x, game_id[:field_w], cp("neutral") | curses.A_REVERSE)

        stdscr.move(field_y, field_x + min(len(game_id), field_w))
        stdscr.refresh()

        # Read one character at a time for inline editing
        key = stdscr.getch()
        if key in (curses.KEY_ENTER, ord('\n'), ord('\r')):
            chosen = game_id.strip() or "local_game"
            curses.noecho()
            curses.curs_set(0)
            return chosen
        elif key in (curses.KEY_BACKSPACE, 127, 8):
            game_id = game_id[:-1]
        elif 32 <= key < 127:  # printable ASCII only
            if len(game_id) < field_w - 1:
                game_id += chr(key)


def render(stdscr, conn: GameConnection, flash_msg: list, game_id: str = "local_game"):
    stdscr.erase()
    h, w = stdscr.getmaxyx()

    with conn._lock:
        state = conn.last_state
        status = conn.status_msg
        connected = conn.connected

    # Title bar
    title = f"       ECO CITY GAME — Dashboard  [{game_id}]  "
    safe_addstr(stdscr, 0, 0, " " * w, cp("title") | curses.A_REVERSE)
    safe_addstr(stdscr, 0, max(0, (w - len(title)) // 2), title,
                cp("title") | curses.A_REVERSE | curses.A_BOLD)

    # Status bar
    status_attr = cp("good") if connected else cp("warn")
    dot = "●" if connected else "○"
    safe_addstr(stdscr, 1, 1, f"{dot} {status}", status_attr)

    # Flash message
    if flash_msg:
        safe_addstr(stdscr, 1, w - len(flash_msg[0]) - 2, flash_msg[0], cp("warn") | curses.A_BOLD)

    if not state:
        safe_addstr(stdscr, h // 2, max(0, w // 2 - 18),
                    "Waiting for first GameState from backend…", cp("dim"))
        _draw_help(stdscr, h, w)
        stdscr.refresh()
        return

    col1_x = 2
    col2_x = w // 2 + 1

    # Resources panel – size dynamically to fit every resource row.
    # 3 = top border + header row + bottom border
    num_resources = max(len(resource_cache), 5)  # at least 5 rows reserved
    panel_h = num_resources + 3
    draw_box(stdscr, 3, col1_x, panel_h, w // 2 - 2, "RESOURCES")

    safe_addstr(stdscr, 4, col1_x + 2,
                f"{'Resource':<22}{'Amount':>12}",
                cp("header") | curses.A_UNDERLINE)

    for k, v in state.resources.items():
        resource_cache[k] = v

    row_y = 5
    for i, (rtype, ramount) in enumerate(sorted(resource_cache.items())):
        if row_y + i >= 3 + panel_h - 1:
            break

        rname = RESOURCE_NAMES.get(rtype, f"Resource {rtype}")
        rclr  = resource_color(rtype, ramount)

        safe_addstr(stdscr, row_y + i, col1_x + 2,
                    f"{'►':1} {rname:<20}", cp("neutral"))

        safe_addstr(stdscr, row_y + i, col1_x + 25,
                    f"{ramount:>10,}", cp(rclr) | curses.A_BOLD)

    # Buildings panel
    bld_start_y = 3 + panel_h + 1
    bld_h = min(len(state.building_counts) + 3, h - bld_start_y - 4)

    if bld_h >= 4 and state.building_counts:
        draw_box(stdscr, bld_start_y, col1_x, bld_h, w // 2 - 2, "BUILDINGS")

        safe_addstr(stdscr, bld_start_y + 1, col1_x + 2,
                    f"{'Building':<30}{'Count':>6}",
                    cp("header") | curses.A_UNDERLINE)
        for i, (btype, count) in enumerate(sorted(state.building_counts.items())):
            if i + 2 >= bld_h - 1:
                break

            bname = BUILDING_NAMES.get(btype, f"Building {btype}")

            safe_addstr(stdscr, bld_start_y + 2 + i, col1_x + 2,
                        f"  {bname:<28}{count:>6}", cp("neutral"))

    # Petition panel
    pet = state.current_petition
    pet_h = 14
    draw_box(stdscr, 3, col2_x, pet_h, w - col2_x - 2, "CURRENT PETITION")

    if pet and pet.building:
        b = pet.building
        bname = BUILDING_NAMES.get(b.type, f"Building {b.type}")
        safe_addstr(stdscr, 4, col2_x + 2, f"ID:    {pet.id}", cp("neutral"))
        safe_addstr(stdscr, 5, col2_x + 2, f"Build: ", cp("neutral"))
        safe_addstr(stdscr, 5, col2_x + 9, bname, cp("warn") | curses.A_BOLD)
        safe_addstr(stdscr, 6, col2_x + 2, f"Cost:  {b.build_cost:,} (× 100k £)", cp("neutral"))
        safe_addstr(stdscr, 7, col2_x + 2, f"Ticks: {b.ticks_to_complete}", cp("neutral"))

        safe_addstr(stdscr, 8, col2_x + 2, "Effects:", cp("header") | curses.A_UNDERLINE)
        for i, eff in enumerate(b.effects):
            if 9 + i >= 3 + pet_h - 1:
                break
            sign  = "+" if eff.delta_value >= 0 else ""
            rname = RESOURCE_NAMES.get(eff.resource_type, f"Res{eff.resource_type}")
            clr   = "good" if eff.delta_value >= 0 else "bad"
            # CO2 is opposite
            if eff.resource_type == 5:
                clr = "bad" if eff.delta_value >= 0 else "good"
            safe_addstr(stdscr, 9 + i, col2_x + 4,
                        f"{rname:<22} {sign}{eff.delta_value:>6}", cp(clr))
    else:
        safe_addstr(stdscr, 5, col2_x + 2, "No active petition.", cp("dim"))

    # Action buttons
    btn_y = 3 + pet_h + 1
    draw_box(stdscr, btn_y, col2_x, 5, w - col2_x - 2, "ACTIONS")
    safe_addstr(stdscr, btn_y + 1, col2_x + 2, "[ A ]", cp("accept") | curses.A_BOLD)
    safe_addstr(stdscr, btn_y + 1, col2_x + 8, " Accept petition", cp("good"))
    safe_addstr(stdscr, btn_y + 2, col2_x + 2, "[ R ]", cp("reject") | curses.A_BOLD)
    safe_addstr(stdscr, btn_y + 2, col2_x + 8, " Reject petition", cp("bad"))
    safe_addstr(stdscr, btn_y + 3, col2_x + 2, "[ S ]", cp("save") | curses.A_BOLD)
    safe_addstr(stdscr, btn_y + 3, col2_x + 8, " Save game", cp("neutral"))

    _draw_help(stdscr, h, w)
    stdscr.refresh()


def _draw_help(stdscr, h, w):
    help_text = "  A=Accept  R=Reject  S=Save  Q=Quit  "
    safe_addstr(stdscr, h - 1, 0, " " * w, cp("dim") | curses.A_REVERSE)
    safe_addstr(stdscr, h - 1, 0, help_text, cp("dim") | curses.A_REVERSE)


def main(stdscr):
    curses.curs_set(0)
    stdscr.nodelay(True)
    stdscr.timeout(50)
    init_colors()

    # Ask the player which save to load on startup
    game_id = render_new_game_prompt(stdscr, "local_game")

    conn = GameConnection(game_id=game_id)
    flash_msg = []
    flash_until = 0.0
    last_state = None

    try:
        while True:
            now = time.time()
            if now > flash_until:
                flash_msg.clear()

            # Check for game over before processing regular input
            with conn._lock:
                is_game_over = conn.game_over
                current_state = conn.last_state

            if is_game_over:
                conn.stop()
                want_quit = render_game_over(stdscr, conn)
                if want_quit:
                    return

                # Player chose N: let them pick a new game ID
                game_id = render_new_game_prompt(stdscr, game_id)

                # Reset and reconnect
                resource_cache.clear()
                conn = GameConnection(game_id=game_id)
                flash_msg = []
                flash_until = 0.0
                last_state = None
                stdscr.nodelay(True)
                stdscr.timeout(50)
                continue

            if last_state is None or current_state is not last_state or flash_msg:
                render(stdscr, conn, flash_msg, game_id)
                last_state = current_state

            key = stdscr.getch()
            if key == curses.ERR:
                continue

            ch = chr(key).lower() if 0 < key < 256 else ""

            if ch == "q":
                break

            elif ch == "a":
                flash_msg[:] = ["Petition accepted – sent to backend"] \
                    if conn.send_action(accept=True) \
                    else ["Not connected – action not sent"]
                flash_until = time.time() + 3

            elif ch == "r":
                flash_msg[:] = ["Petition rejected – sent to backend"] \
                    if conn.send_action(accept=False) \
                    else ["Not connected – action not sent"]
                flash_until = time.time() + 3

            elif ch == "s":
                flash_msg[:] = ["Save command sent to backend"] \
                    if conn.send_action(save=True) \
                    else ["Not connected – action not sent"]
                flash_until = time.time() + 3

    finally:
        conn.stop()

if __name__ == "__main__":
    curses.wrapper(main)
