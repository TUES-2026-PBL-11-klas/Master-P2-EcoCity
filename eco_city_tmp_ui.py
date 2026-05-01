#!/usr/bin/env python3

# Eco City Game – Python TUI temporary UI for testing the C++ backend
# Connects to the C++ backend on TCP port 54321
# Protocol: 4-byte big-endian length header + protobuf-encoded body
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


def encode_ui_action(accept: Optional[bool] = None, save: bool = False) -> bytes:
    # UIAction:
    # field 1: PetitionResponse { field 1: responded (bool), field 2: accepted (bool) }
    # field 2: save_game (bool)

    body = b""
    if accept is not None:
        pr = _encode_field_bool(1, True) + _encode_field_bool(2, accept)
        body += _encode_field_bytes(1, pr)
    if save:
        body += _encode_field_bool(2, True)
    return body


HOST = "127.0.0.1"
PORT = 7777
RECONNECT_DELAY = 2.0


class GameConnection:
    def __init__(self):
        self._sock: Optional[socket.socket] = None
        self._lock = threading.Lock()
        self.connected = False
        self.last_state: Optional[GameState] = None
        self.status_msg = "Connecting…"
        self._running = True
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
                header = self._recvall(sock, 4)
                if not header:
                    with self._lock:
                        self.status_msg = "Disconnected – waiting for backend…"
                    return
                length = struct.unpack(">I", header)[0]
                if length == 0 or length > 1_048_576:
                    return
                payload = self._recvall(sock, length)
                if not payload:
                    return
                try:
                    gs = parse_game_state(payload)

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


    def send_action(self, accept: Optional[bool] = None, save: bool = False):
        payload = encode_ui_action(accept=accept, save=save)
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


def render(stdscr, conn: GameConnection, flash_msg: list):
    stdscr.erase()
    h, w = stdscr.getmaxyx()

    with conn._lock:
        state = conn.last_state
        status = conn.status_msg
        connected = conn.connected

    # Title bar
    title = "       ECO CITY GAME — Dashboard  "
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

    # Resources panel
    panel_h = 9
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

    conn = GameConnection()
    flash_msg = []
    flash_until = 0.0

    last_state = None

    try:
        while True:
            now = time.time()
            if now > flash_until:
                flash_msg.clear()

            # Only redraw when something changes
            with conn._lock:
                current_state = conn.last_state

            if last_state is None or current_state is not last_state or flash_msg:
                render(stdscr, conn, flash_msg)
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
