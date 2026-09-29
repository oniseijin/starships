#!/usr/bin/env python3
"""Tests for the Starships web server (v0.1.2 parity surface).

Stdlib unittest only; no network. The WebSocket handler is exercised with a
fake connection object, and the simulation directly through Game/Ship.

Covers: the green-ship skin + input gating (v0.1.1), the `m` lazer-color
menu (shared mutable color recolors in-flight lazers; any client may set
either ship; clamping/validation), OS key-repeat fire (counted fire actions,
11-lazer cap), hotseat/remote slot arbitration (single client drives both
ships, second client claims B, B's disconnect returns hotseat, Ship A mouse
buttons from any client), and global restart semantics (both ships reset,
colors + skin survive, fresh process = defaults).

Run:  python3 web/test_server.py
"""

import asyncio
import json
import sys
import types
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

# server.py imports `websockets` at module level, but the sim logic under
# test never needs it — stub it when this interpreter doesn't have the
# package so the tests run with a bare python3.
try:
    import websockets  # noqa: F401
except ImportError:
    _stub = types.ModuleType("websockets")

    class ConnectionClosed(Exception):
        pass

    _stub.ConnectionClosed = ConnectionClosed
    sys.modules["websockets"] = _stub

from server import START_HULL, Clients, Game, route_input, ws_handler


class OneShotWS:
    """Minimal fake websocket: yields the given frames, then closes.

    Collects everything the handler sends back (hello, snapshots) in .sent.
    """

    def __init__(self, incoming=()):
        self.incoming = list(incoming)
        self.sent = []

    async def send(self, data):
        self.sent.append(data)

    def __aiter__(self):
        return self

    async def __anext__(self):
        if self.incoming:
            return self.incoming.pop(0)
        raise StopAsyncIteration


def run_handler(ws, game, clients):
    asyncio.run(asyncio.wait_for(ws_handler(ws, game, clients), 5))


class GreenSkinSimTests(unittest.TestCase):
    """The g action at the Game/Ship level (slot = connection ownership)."""

    def setUp(self):
        self.game = Game()

    def tick_and_snapshot(self):
        self.game.tick()
        return json.loads(self.game.snapshot())

    def test_fresh_game_starts_unskinned(self):
        snap = self.tick_and_snapshot()
        self.assertEqual(snap["a"]["skin"], "normal")
        self.assertEqual(snap["b"]["skin"], "normal")

    def test_snapshot_includes_skin_field_for_both_ships(self):
        snap = json.loads(self.game.snapshot())
        self.assertIn("skin", snap["a"])
        self.assertIn("skin", snap["b"])

    def test_green_from_ship_a_slot_sets_skin(self):
        self.game.apply_input("a", {"t": "input", "green": True})
        snap = self.tick_and_snapshot()
        self.assertEqual(snap["a"]["skin"], "green")
        self.assertEqual(snap["b"]["skin"], "normal")

    def test_green_from_ship_b_slot_ignored(self):
        self.game.apply_input("b", {"t": "input", "green": True})
        snap = self.tick_and_snapshot()
        self.assertEqual(snap["a"]["skin"], "normal")
        self.assertEqual(snap["b"]["skin"], "normal")

    def test_green_is_one_way_no_switch_back(self):
        self.game.apply_input("a", {"t": "input", "green": True})
        self.tick_and_snapshot()
        # pressing g again / a green:false flag must not un-skin
        self.game.apply_input("a", {"t": "input", "green": False})
        snap = self.tick_and_snapshot()
        self.assertEqual(snap["a"]["skin"], "green")

    def test_restart_preserves_green(self):
        self.game.apply_input("a", {"t": "input", "green": True})
        self.tick_and_snapshot()
        self.game.a.hull = 1  # something for restart to reset
        self.game.apply_input("a", {"t": "input", "restart": True})
        snap = self.tick_and_snapshot()
        self.assertEqual(snap["a"]["hull"], START_HULL)  # restart took effect
        self.assertEqual(snap["a"]["skin"], "green")     # and skin survived

    def test_green_applies_even_when_ship_dead(self):
        # C++ keyPressed('g') runs regardless of ship state
        self.game.a.hull = 0
        self.game.a.active = False
        self.game.apply_input("a", {"t": "input", "green": True})
        snap = self.tick_and_snapshot()
        self.assertEqual(snap["a"]["skin"], "green")

    def test_green_survives_many_ticks(self):
        self.game.apply_input("a", {"t": "input", "green": True})
        self.tick_and_snapshot()
        for _ in range(120):
            self.game.tick()
        self.game.pending["a"]["restart"] = True
        self.game.tick()
        snap = json.loads(self.game.snapshot())
        self.assertEqual(snap["a"]["skin"], "green")


class ConnectionOwnershipTests(unittest.TestCase):
    """g must only act when it arrives on Ship A's own connection."""

    def test_register_hands_out_a_then_b_then_spectator(self):
        clients = Clients()
        s1, s2, s3 = object(), object(), object()
        self.assertEqual(clients.register(s1), "a")
        self.assertEqual(clients.register(s2), "b")
        self.assertIsNone(clients.register(s3))
        clients.unregister(s1)
        self.assertEqual(clients.register(s3), "a")  # freed slot is reused

    def test_g_from_ship_a_connection_sets_skin_in_snapshots(self):
        game, clients = Game(), Clients()
        ws_a = OneShotWS([json.dumps({"t": "input", "green": True})])
        run_handler(ws_a, game, clients)
        self.assertEqual(json.loads(ws_a.sent[0])["player"], "A")
        game.tick()
        snap = json.loads(game.snapshot())
        self.assertEqual(snap["a"]["skin"], "green")
        self.assertEqual(snap["b"]["skin"], "normal")

    def test_g_from_ship_b_connection_ignored(self):
        game, clients = Game(), Clients()
        clients.register(object())  # a live connection owns Ship A
        ws_b = OneShotWS([json.dumps({"t": "input", "green": True})])
        run_handler(ws_b, game, clients)
        self.assertEqual(json.loads(ws_b.sent[0])["player"], "B")
        game.tick()
        snap = json.loads(game.snapshot())
        self.assertEqual(snap["a"]["skin"], "normal")
        self.assertEqual(snap["b"]["skin"], "normal")

    def test_g_from_spectator_ignored(self):
        game, clients = Game(), Clients()
        clients.register(object())  # Ship A
        clients.register(object())  # Ship B
        ws_s = OneShotWS([json.dumps({"t": "input", "green": True})])
        run_handler(ws_s, game, clients)
        self.assertEqual(json.loads(ws_s.sent[0])["player"], "?")
        game.tick()
        snap = json.loads(game.snapshot())
        self.assertEqual(snap["a"]["skin"], "normal")
        self.assertEqual(snap["b"]["skin"], "normal")

    def test_regular_inputs_still_work_from_ship_b(self):
        # the green gating must not have broken Ship B's own controls
        game, clients = Game(), Clients()
        clients.register(object())  # Ship A
        ws_b = OneShotWS([json.dumps({"t": "input", "right": True})])
        run_handler(ws_b, game, clients)
        rot_before = game.b.rot
        game.tick()
        game.tick()  # rotation applies on frame % 2
        self.assertAlmostEqual(game.b.rot, rot_before + 0.07)
        snap = json.loads(game.snapshot())
        self.assertEqual(snap["a"]["skin"], "normal")


class LazerColorTests(unittest.TestCase):
    """D1: the `m` menu color-set action — one shared mutable color per ship,
    app-global (any client may set either), recolors in-flight lazers."""

    def setUp(self):
        self.game = Game()

    def snap(self):
        self.game.tick()
        return json.loads(self.game.snapshot())

    def test_fresh_process_defaults(self):
        self.assertEqual(self.game.a.color, [127, 229, 238])
        self.assertEqual(self.game.b.color, [255, 192, 204])
        snap = json.loads(self.game.snapshot())
        self.assertEqual(snap["a"]["col"], [127, 229, 238])
        self.assertEqual(snap["b"]["col"], [255, 192, 204])

    def test_color_action_sets_ship_color(self):
        self.game.apply_color({"ship": "b", "r": 10, "g": 20, "b": 30})
        snap = self.snap()
        self.assertEqual(snap["b"]["col"], [10, 20, 30])
        self.assertEqual(snap["a"]["col"], [127, 229, 238])  # other ship untouched

    def test_recolor_hits_in_flight_lazers(self):
        # the C++ quirk: lazers hold the ship's ONE rgb object, so already-
        # in-flight lazers change color the moment the slider moves
        self.game.a.x, self.game.a.y = 700.0, 700.0  # park away from B
        self.game.b.x, self.game.b.y = 200.0, 200.0
        self.game.apply_input("a", {"fire": True})
        snap = self.snap()
        self.assertEqual(len(snap["L"]), 1)
        self.assertEqual(snap["L"][0]["c"], [127, 229, 238])
        self.game.apply_color({"ship": "a", "r": 255, "g": 0, "b": 0})
        snap = self.snap()
        self.assertEqual(len(snap["L"]), 1)  # same lazer, still alive
        self.assertEqual(snap["L"][0]["c"], [255, 0, 0])
        # mechanism: the lazer references the ship's shared color object
        self.assertIs(self.game.a.lazers[0].color, self.game.a.color)

    def test_color_settable_from_any_client(self):
        # the C++ panel is app-global: B's connection and even spectators
        # may set either ship's color; last write wins
        game, clients = Game(), Clients()
        clients.register(object())  # Ship A's connection
        ws_b = OneShotWS([json.dumps(
            {"t": "color", "ship": "a", "r": 1, "g": 2, "b": 3})])
        run_handler(ws_b, game, clients)
        self.assertEqual(json.loads(ws_b.sent[0])["player"], "B")
        self.assertEqual(game.a.color, [1, 2, 3])
        clients.register(object())  # Ship B's connection
        ws_s = OneShotWS([json.dumps(
            {"t": "color", "ship": "b", "r": 9, "g": 8, "b": 7})])
        run_handler(ws_s, game, clients)
        self.assertEqual(json.loads(ws_s.sent[0])["player"], "?")  # spectator
        self.assertEqual(game.b.color, [9, 8, 7])

    def test_color_values_clamped_and_invalid_ignored(self):
        self.game.apply_color({"ship": "a", "r": 300, "g": -5, "b": 12.7})
        self.assertEqual(self.game.a.color, [255, 0, 12])
        before = list(self.game.a.color)
        self.game.apply_color({"ship": "a"})  # missing channels
        self.game.apply_color({"ship": "z", "r": 1, "g": 2, "b": 3})  # no ship z
        self.game.apply_color({"ship": "a", "r": "x", "g": 2, "b": 3})
        self.assertEqual(self.game.a.color, before)


class RepeatFireTests(unittest.TestCase):
    """D3 MUST: C++ re-fires on OS key-repeat — holding fire auto-fires.
    Fire actions are counted server-side; the 11-lazer cap still applies."""

    def setUp(self):
        self.game = Game()
        self.game.a.x, self.game.a.y = 700.0, 700.0  # park away from B
        self.game.b.x, self.game.b.y = 200.0, 200.0

    def fire(self):
        self.game.apply_input("a", {"fire": True})
        self.game.tick()

    def test_two_fire_actions_two_lazers(self):
        self.fire()
        self.fire()
        self.assertEqual(len(self.game.a.lazers), 2)

    def test_holding_fire_honors_11_lazer_cap(self):
        for _ in range(12):  # 12 quick actions: the size() <= MAX quirk caps at 11
            self.fire()
        self.assertEqual(len(self.game.a.lazers), 11)

    def test_shield_actions_counted_as_toggles(self):
        self.game.apply_input("a", {"shield": True})
        self.game.apply_input("a", {"shield": True})
        self.game.tick()
        # counted, not folded: two toggles cancel out. A boolean flag would
        # have collapsed both messages into ONE toggle (= on).
        self.assertFalse(self.game.a.shield_on)


class RestartTests(unittest.TestCase):
    """r is global (ofApp::restart resets BOTH ships); colors + skin survive;
    a fresh process starts at defaults."""

    def test_restart_is_global(self):
        game = Game()
        game.a.hull = 2
        game.b.hull = 3
        game.b.active = False
        game.apply_input("b", {"restart": True})  # from B's connection alone
        game.tick()
        self.assertEqual(game.a.hull, START_HULL)
        self.assertEqual(game.b.hull, START_HULL)
        self.assertTrue(game.b.active)

    def test_restart_keeps_colors_and_skin(self):
        game = Game()
        game.apply_color({"ship": "a", "r": 1, "g": 2, "b": 3})
        game.apply_color({"ship": "b", "r": 4, "g": 5, "b": 6})
        game.apply_input("a", {"green": True})
        game.a.hull = 1
        game.apply_input("a", {"restart": True})
        game.tick()
        self.assertEqual(game.a.hull, START_HULL)   # restart took effect...
        self.assertEqual(game.a.color, [1, 2, 3])   # ...colors survived...
        self.assertEqual(game.b.color, [4, 5, 6])
        self.assertEqual(game.a.skin, "green")      # ...and so did the skin

    def test_fresh_process_defaults(self):
        game = Game()
        self.assertEqual(game.a.skin, "normal")
        self.assertEqual(game.b.skin, "normal")
        self.assertEqual(game.a.color, [127, 229, 238])
        self.assertEqual(game.b.color, [255, 192, 204])


class HotseatRoutingTests(unittest.TestCase):
    """D2 slot arbitration at the routing level: hotseat (one client flies
    both), takeover by a second client, hotseat again when B leaves."""

    def setUp(self):
        self.clients = Clients()

    def route(self, sender, msg):
        return route_input(self.clients, sender, msg)

    def test_single_client_drives_both_ships(self):
        ws1 = object()
        self.clients.register(ws1)
        self.assertEqual(self.clients.mode(), "hotseat")
        self.assertEqual(self.route(ws1, {"ship": "a", "left": True}), "a")
        self.assertEqual(self.route(ws1, {"ship": "b", "right": True}), "b")
        self.assertEqual(self.route(ws1, {"left": True}), "a")  # untagged -> own

    def test_second_client_claims_b_first_client_loses_b(self):
        ws1, ws2 = object(), object()
        self.clients.register(ws1)
        self.clients.register(ws2)
        self.assertEqual(self.clients.mode(), "remote")
        # ws1's B-keyset no longer drives B — it falls back to its own ship
        self.assertEqual(self.route(ws1, {"ship": "b", "right": True}), "a")
        # and ws2 flies its own ship with either keyset (v0.1.1 rule)
        self.assertEqual(self.route(ws2, {"ship": "a", "left": True}), "b")
        self.assertEqual(self.route(ws2, {"left": True}), "b")

    def test_b_disconnect_returns_hotseat(self):
        ws1, ws2 = object(), object()
        self.clients.register(ws1)
        self.clients.register(ws2)
        self.clients.unregister(ws2)
        self.assertEqual(self.clients.mode(), "hotseat")
        self.assertEqual(self.route(ws1, {"ship": "b", "left": True}), "b")

    def test_spectator_input_dropped(self):
        self.clients.register(object())
        self.clients.register(object())
        self.assertIsNone(self.route(object(), {"left": True}))
        self.assertIsNone(self.route(object(), {"ship": "a", "fire": True}))

    def test_buttons_drive_ship_a_from_any_client_any_mode(self):
        ws1 = object()
        self.clients.register(ws1)
        self.assertEqual(self.route(ws1, {"via": "button", "thrust": True}), "a")
        self.clients.register(object())  # remote now
        self.assertEqual(self.route(ws1, {"via": "button", "left": True}), "a")
        self.assertEqual(self.route(object(), {"via": "button", "fire": True}), "a")

    def test_spectator_only_connection_is_not_hotseat(self):
        self.clients.register(object())
        self.clients.register(object())
        spec = object()
        self.clients.register(spec)
        self.clients.unregister(self.clients.slots["a"])
        self.clients.unregister(self.clients.slots["b"])
        self.assertEqual(self.clients.mode(), "remote")  # lone spectator watches
        self.assertIsNone(self.route(spec, {"ship": "b", "left": True}))


class HotseatHandlerTests(unittest.TestCase):
    """D2 end-to-end through ws_handler with fake sockets."""

    def test_single_client_b_keyset_drives_ship_b(self):
        game, clients = Game(), Clients()
        ws = OneShotWS([json.dumps(
            {"t": "input", "ship": "b", "right": True})])
        run_handler(ws, game, clients)
        welcome = json.loads(ws.sent[0])
        self.assertEqual(welcome["player"], "A")
        self.assertEqual(welcome["mode"], "hotseat")
        game.tick()
        game.tick()  # rotation applies on frame % 2
        self.assertAlmostEqual(game.b.rot, 0.07)
        self.assertEqual(game.a.rot, 0.0)  # ship A was not touched

    def test_color_message_from_sole_client(self):
        game, clients = Game(), Clients()
        ws = OneShotWS([json.dumps(
            {"t": "color", "ship": "b", "r": 50, "g": 60, "b": 70})])
        run_handler(ws, game, clients)
        self.assertEqual(game.b.color, [50, 60, 70])

    def test_welcome_mode_remote_when_two_registered(self):
        clients = Clients()
        clients.register(object())
        clients.register(object())
        ws = OneShotWS([])
        run_handler(ws, Game(), clients)
        self.assertEqual(json.loads(ws.sent[0])["mode"], "remote")

    def test_snapshot_carries_mode_and_colors(self):
        snap = json.loads(Game().snapshot("hotseat"))
        self.assertEqual(snap["mode"], "hotseat")
        self.assertIn("col", snap["a"])
        self.assertIn("col", snap["b"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
