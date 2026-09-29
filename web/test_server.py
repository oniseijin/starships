#!/usr/bin/env python3
"""Tests for the Starships web server — the green-ship skin and input gating.

Stdlib unittest only; no network. The WebSocket handler is exercised with a
fake connection object, and the simulation directly through Game/Ship.

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

from server import START_HULL, Clients, Game, ws_handler


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


if __name__ == "__main__":
    unittest.main(verbosity=2)
