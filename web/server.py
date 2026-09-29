#!/usr/bin/env python3
"""Starships web 1v1 — LAN server (rules-faithful remake of the oF game).

Host-authoritative: the server runs the 60 Hz simulation (the same rules and
constants as src/*.cpp from the original openFrameworks game) and broadcasts
state at 30 Hz. Two browsers on the LAN each control one ship.

Run:
    uv run --python 3.13 --with websockets web/server.py
    (or: web/run.sh)

Then open the printed URL on two machines on the same LAN.
First tab connected = Ship A (cyan), second = Ship B (purple), extras watch.
"""

import asyncio
import json
import math
import socket
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import websockets

REPO_ROOT = Path(__file__).resolve().parent.parent
STATIC_DIR = Path(__file__).resolve().parent / "static"
ASSETS_DIR = REPO_ROOT / "bin" / "data"  # original game art/sounds, served as /assets/

PORT = 47777

# ---- faithful constants from src/Ship.h, Lazer.h, Shield.h, Sizes.h, ofApp.cpp
W, H = 1500, 900              # Sizes.h MAX_WIDTH / MAX_HEIGHT (web: no control strip)
MAX_SPEED = 10.0              # Ship.h
ROT_STEP = 0.07               # ofApp.cpp increaseRotation per applied frame
THRUST = -0.0125              # ofApp.cpp thruster()
MAX_LAZERS = 10               # Ship.h ("size() <= MAX_LAZERS" quirk kept)
LASER_SPEED = 11.0            # Lazer.h
LASER_LENGTH = 20.0           # Lazer.h
LASER_KEEP_ALIVE = 650        # Lazer.h frames
SELF_HIT_GRACE = 20           # Lazer.cpp framesAlive guard
SHIP_RADIUS = 30              # Lazer.cpp pointBall(..., 30) at (x, y+3)
SHIELD_SIZE = 40              # Shield.h size
SHIELD_MAX = 5.0              # Shield.h STRENGTH_MAX
SHIELD_DECAY = 0.02           # Shield.h DECAY_RATE
SHIELD_CHARGE = 0.01          # Shield.h CHARGE_RATE
START_HULL = 5                # ofApp.cpp restart(): strength = 5
HALF_ACCEL_EVERY = 8          # ofApp.cpp draw(): frame % 8
INPUT_EVERY = 2               # ofApp.cpp: rotation/thrust applied on frame % 2

LAZER_COLOR_A = (127, 229, 238)   # ofApp.cpp aLazerDefault
LAZER_COLOR_B = (255, 192, 204)   # ofApp.cpp bLazerDefault

HALF_PI = math.pi / 2.0
TWO_PI = math.pi * 2.0


class Ship:
    def __init__(self, color):
        self.x = W / 2.0
        self.y = H / 2.0
        self.vx = 0.0
        self.vy = 0.0
        self.ax = 0.0
        self.ay = 0.0
        self.rot = 0.0
        self.hull = START_HULL
        self.active = True
        self.thrust_on = False
        self.shield_on = False
        self.shield_strength = SHIELD_MAX
        self.shield_enabled = True
        self.lazers = []
        self.color = color
        self.skin = "normal"  # 'g' flips Ship A to bird_of_prey (one-way)
        self.events = []  # drained into the broadcast events list

    def restart(self):
        # skin survives: ofApp::restart() re-renders from baseImage
        self.hull = START_HULL
        self.active = True
        self.thrust_on = False

    def set_green(self):
        # ofApp.cpp keyPressed('g'): baseImage = SHIP_GREEN_IMAGE and BOTH the
        # idle and thrust textures load that SAME file (no green thrust art —
        # thrusting draws the identical sprite). Idempotent and one-way:
        # pressing g again just reloads the same texture; only a fresh launch
        # starts unskinned.
        self.skin = "green"

    def update(self):
        # physics (Ship::update)
        self.vx += self.ax
        self.vy += self.ay
        sp = math.hypot(self.vx, self.vy)
        if sp > MAX_SPEED:
            self.vx *= MAX_SPEED / sp
            self.vy *= MAX_SPEED / sp
        self.x += self.vx
        self.y += self.vy
        self._wrap()
        # shield (Shield::update)
        if self.shield_on:
            self.shield_strength -= SHIELD_DECAY
        if self.shield_strength < SHIELD_MAX:
            self.shield_strength += SHIELD_CHARGE
        if self.shield_strength < 0.0:
            self.shield_enabled = False
            self.shield_on = False
        if self.shield_strength > SHIELD_MAX:
            self.shield_strength = SHIELD_MAX
            self.shield_enabled = True
        # lazers: update own, then collisions vs self from own + opponent's
        # (Ship::update loops own lazers with update(), opponent's without)
        for lazer in self.lazers:
            lazer.update()
            self._check_lazer(lazer)
        for lazer in self.opponent.lazers:
            self._check_lazer(lazer)
        self.lazers = [l for l in self.lazers if l.active]

    def _wrap(self):
        if self.x < 0:
            self.x = W
        if self.x > W:
            self.x = 0
        if self.y < 0:
            self.y = H
        if self.y > H:
            self.y = 0

    def _check_lazer(self, lazer):
        if not lazer.active:
            return
        # shield circle first (Lazer::collision(Shield*)); silent absorb
        if self.shield_on and self._points_hit(
            lazer, self.x, self.y, SHIELD_SIZE
        ):
            if not (lazer.frames_alive < SELF_HIT_GRACE and lazer.ship is self):
                self.shield_strength -= 1.0
                lazer.active = False
            return
        # ship circle (Lazer::collision(Ship*)): r=30 at (x, y+3)
        if self.active and self._points_hit(lazer, self.x, self.y + 3, SHIP_RADIUS):
            if lazer.frames_alive < SELF_HIT_GRACE and lazer.ship is self:
                return
            if self.shield_on:  # faithful: Shield::handleCollision path
                self.shield_strength -= 1.0
            else:
                self.hull -= 1
                self.events.append("hit")
            lazer.active = False
            if self.hull <= 0 and self.active:
                self.active = False
                self.events.append("explode")

    def _points_hit(self, lazer, bx, by, diameter):
        r = diameter / 2.0
        for px, py in ((lazer.x, lazer.y), (lazer.ex, lazer.ey)):
            if math.hypot(px - bx, py - by) < r:
                return True
        return False

    # ---- inputs (ofApp::handleControls / keyPressed)
    def rotate(self, sign):
        if not self.active:
            return
        self.rot += sign * ROT_STEP
        if self.rot > TWO_PI:
            self.rot -= TWO_PI
        if self.rot < 0:
            self.rot += TWO_PI

    def thrust(self):
        if not self.active:
            return
        r = self.rot - HALF_PI
        # C++ thruster(): f1=cos(r)*t, f2=sin(r)*t, both negated -> ax += -cos(r)*t
        self.ax += -math.cos(r) * THRUST
        self.ay += -math.sin(r) * THRUST

    def fire(self):
        if not self.active:
            return
        if len(self.lazers) <= MAX_LAZERS:  # faithful off-by-one quirk
            angle = self.rot - HALF_PI
            self.lazers.append(
                Lazer(self, self.x, self.y, math.cos(angle) * LASER_SPEED,
                      math.sin(angle) * LASER_SPEED, angle, self.color)
            )
            self.events.append("fire")

    def toggle_shield(self):
        if self.shield_enabled:
            self.shield_on = not self.shield_on

    def half_accel(self):
        if self.ax != 0.0:
            self.ax /= 2
        if self.ay != 0.0:
            self.ay /= 2


class Lazer:
    def __init__(self, ship, x, y, vx, vy, angle, color):
        self.ship = ship
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy
        self.angle = angle
        self.ex = x
        self.ey = y
        self.frames_alive = 0
        self.active = True
        self.color = color

    def update(self):
        self.frames_alive += 1
        if self.frames_alive > LASER_KEEP_ALIVE:
            self.active = False
        self.x += self.vx
        self.y += self.vy
        if self.x < 0:
            self.x = W
        if self.x > W:
            self.x = 0
        if self.y < 0:
            self.y = H
        if self.y > H:
            self.y = 0
        self.ex = self.x + math.cos(self.angle) * LASER_LENGTH
        self.ey = self.y + math.sin(self.angle) * LASER_LENGTH


class Game:
    """Server-authoritative 60 Hz sim mirroring ofApp::update/draw ordering:
    physics+collisions -> inputs -> (every 8 frames) half-accel."""

    def __init__(self):
        self.a = Ship(LAZER_COLOR_A)
        self.b = Ship(LAZER_COLOR_B)
        self.a.opponent = self.b
        self.b.opponent = self.a
        self.frame = 0
        self.pending = {"a": self._blank_input(), "b": self._blank_input()}
        self.events = []

    @staticmethod
    def _blank_input():
        return {"left": False, "right": False, "thrust": False,
                "fire": False, "shield": False, "restart": False,
                "green": False}

    def apply_input(self, slot, msg):
        cur = self.pending[slot]
        for k in ("left", "right", "thrust"):
            if k in msg:
                cur[k] = bool(msg[k])
        for k in ("fire", "shield", "restart"):  # edge-triggered actions
            if msg.get(k):
                cur[k] = True
        if slot == "a" and msg.get("green"):
            # 'g' is Ship A only, and only from the connection that owns
            # Ship A (spectators / Ship B pressing g do nothing)
            cur["green"] = True

    def tick(self):
        self.frame += 1
        self.a.update()
        self.b.update()
        for slot, ship in (("a", self.a), ("b", self.b)):
            inp = self.pending[slot]
            if self.frame % INPUT_EVERY == 0:
                if inp["left"]:
                    ship.rotate(-1)
                if inp["right"]:
                    ship.rotate(1)
                if inp["thrust"]:
                    ship.thrust()
                    ship.thrust_on = True
                else:
                    ship.thrust_on = False
            if inp["fire"]:
                ship.fire()
                inp["fire"] = False
            if inp["shield"]:
                ship.toggle_shield()
                inp["shield"] = False
            if inp["restart"]:
                ship.restart()
                inp["restart"] = False
            if inp["green"]:
                ship.set_green()
                inp["green"] = False
        if self.frame % HALF_ACCEL_EVERY == 0:
            self.a.half_accel()
            self.b.half_accel()

    def snapshot(self):
        events = self.events
        self.events = []
        for ship in (self.a, self.b):
            events.extend(ship.events)
            ship.events = []

        def ship_state(s):
            return {
                "x": round(s.x, 1), "y": round(s.y, 1),
                "r": round(s.rot, 3),
                "hull": s.hull, "active": s.active, "th": s.thrust_on,
                "sh": s.shield_on and s.shield_enabled,
                "shStr": round(s.shield_strength, 2),
                "skin": s.skin,
            }

        return json.dumps({
            "t": "s", "f": self.frame,
            "a": ship_state(self.a), "b": ship_state(self.b),
            "L": [
                {"x": round(l.x, 1), "y": round(l.y, 1),
                 "x2": round(l.ex, 1), "y2": round(l.ey, 1), "c": l.color}
                for l in self.a.lazers + self.b.lazers if l.active
            ],
            "e": events,
        })


class Clients:
    """Connection registry: first ws = ship A, second = ship B, rest watch."""

    def __init__(self):
        self.slots = {"a": None, "b": None}
        self.all = set()

    def register(self, ws):
        self.all.add(ws)
        for name in ("a", "b"):
            if self.slots[name] is None:
                self.slots[name] = ws
                return name
        return None

    def unregister(self, ws):
        self.all.discard(ws)
        for name, conn in self.slots.items():
            if conn is ws:
                self.slots[name] = None


async def ws_handler(websocket, game, clients):
    slot = clients.register(websocket)
    player = {"a": "A", "b": "B", None: "?"}[slot]
    hello = json.dumps({"t": "welcome", "player": player})
    await websocket.send(hello)
    print(f"client connected as Ship {player} ({len(clients.all)} connected)")
    try:
        async for raw in websocket:
            try:
                msg = json.loads(raw)
            except (ValueError, TypeError):
                continue
            if msg.get("t") == "input" and slot:
                game.apply_input(slot, msg)
    except websockets.ConnectionClosed:
        pass
    finally:
        clients.unregister(websocket)
        print(f"client disconnected (Ship {player})")


def lan_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("10.255.255.255", 1))  # no packets sent; just picks a route
        return s.getsockname()[0]
    except OSError:
        return "127.0.0.1"
    finally:
        s.close()


class StaticHandler(BaseHTTPRequestHandler):
    ASSET_EXTS = {".png", ".jpg", ".jpeg", ".mp3", ".ico", ".svg"}

    def log_message(self, fmt, *args):
        pass  # keep the console readable while the sim runs

    def _serve(self, path, content_type):
        try:
            data = path.read_bytes()
        except OSError:
            self.send_error(404)
            return
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-cache")
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        route = self.path.split("?")[0]
        if route in ("/", "/index.html"):
            self._serve(STATIC_DIR / "index.html", "text/html; charset=utf-8")
        elif route == "/client.js":
            self._serve(STATIC_DIR / "client.js",
                        "text/javascript; charset=utf-8")
        elif route.startswith("/assets/"):
            name = Path(route).name  # basename only: no traversal
            path = ASSETS_DIR / name
            if path.suffix.lower() in self.ASSET_EXTS:
                ctype = {".png": "image/png", ".jpg": "image/jpeg",
                         ".jpeg": "image/jpeg", ".mp3": "audio/mpeg",
                         ".ico": "image/x-icon", ".svg": "image/svg+xml"}[path.suffix.lower()]
                self._serve(path, ctype)
            else:
                self.send_error(404)
        else:
            self.send_error(404)


async def main():
    port = PORT
    if len(sys.argv) > 1:
        port = int(sys.argv[1])

    game = Game()
    clients = Clients()

    httpd = ThreadingHTTPServer(("0.0.0.0", port), StaticHandler)
    threading_serve = asyncio.get_running_loop().run_in_executor(
        None, httpd.serve_forever, 0.05
    )

    async with websockets.serve(
        lambda ws: ws_handler(ws, game, clients), "0.0.0.0", port + 1,
        max_size=64 * 1024, ping_interval=20,
    ) as ws_server:
        ip = lan_ip()
        print("=" * 62)
        print(" STARSHIPS web 1v1")
        print(f"   this Mac : http://localhost:{port}")
        print(f"   LAN      : http://{ip}:{port}   <-- open on both machines")
        print(f"   ws       : ws://{ip}:{port + 1}")
        print("   first tab = Ship A (a/d/s/w/x, g = green skin), second = Ship B (4/6/5/8/2 or l/'/;/p//)")
        print("=" * 62)

        tick_rate = 1 / 60.0
        while True:
            game.tick()
            if game.frame % 2 == 0:  # broadcast at 30 Hz
                snapshot = game.snapshot()
                dead = []
                for ws in list(clients.all):
                    try:
                        await ws.send(snapshot)
                    except websockets.ConnectionClosed:
                        dead.append(ws)
                for ws in dead:
                    clients.unregister(ws)
            await asyncio.sleep(tick_rate)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
