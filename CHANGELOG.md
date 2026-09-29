# CHANGELOG

## [0.1.2] — 2026-09-30

* `web/`: hotseat mode — a single connected browser is the couch game (one
  keyboard flies BOTH ships: A keyset → Ship A, B keysets → Ship B, `r`
  global, `g` Ship A only); a second browser claims Ship B and everyone
  flips to remote 1v1 (either keyset flies your own ship, the v0.1.1 rule);
  Ship B's disconnect returns the remaining client to hotseat. Server
  arbitrates slot ownership; clients render a hotseat/remote HUD indicator.
* `web/`: the `m` lazer-color menu (C++ ofxPanel port) — aLazer (cyan
  127,229,238) + bLazer (pink 255,192,204) pickers, full RGB range, drawn
  at alpha 255 as in `Lazer::display`. Mouse/touch only; the game keeps
  running and all keys still steer while open. Colors are app-global (any
  connected client, last write wins) and survive `r`; fresh process =
  defaults. Replicates the shared-mutable-color quirk: recoloring a ship
  recolors its already-in-flight lazers instantly (one color object per
  ship, referenced by every lazer, serialized at broadcast time).
* `web/` parity: OS key-repeat re-fires fire/shield (holding fire
  auto-fires, holding shield flickers — counted per-action server-side, no
  client repeat guard; `m`/`g`/`r` keep a one-shot guard); arena is the
  C++ 1500×800 verticalBounds (spawn y=400) with the client letterboxing a
  1500×900 canvas and rendering the 100px control strip; Ship A RectButtons
  (shoot/left/right/thruster/shield at the C++ positions) drive Ship A from
  any client in any mode; red hull≤1 ring moved inside the ship's rotated
  transform (offset (0,−7) rotates with the ship); lazer line width 1px
  (was 3); sound volumes fire/hit/explode 1.0, thrust loop 0.5 (C++
  defaults).
* `web/test_server.py`: 34 tests (from 14) covering the color menu (shared
  color object, in-flight recolor, any-client set, clamping/validation),
  repeat-fire (two actions = two lazers, 11-lazer cap), hotseat routing
  (drive both → takeover → hotseat again, buttons from any client, lone
  spectator not hotseat) and global restart semantics (both ships reset,
  colors + skin survive, fresh process = defaults). All green.
* Functional check: 17/17 live ws checks on throwaway ports (menu color-set
  + in-flight recolor over the wire, hotseat transitions, global restart,
  clamp); server torn down cleanly after.

## [0.1.1] — 2026-09-29

* `web/`: green-ship port — `g` switches Ship A to the green Bird-of-Prey
  skin (`bin/data/bird_of_prey.png` served as both the idle and thrust
  frame; there is no green thrust art, faithful to the C++ `g` handler).
  Ship A only: input from Ship B's connection or a spectator is ignored.
  One-way (no switch-back), survives `r` restart, resets on a fresh server
  launch; the simulation is untouched (cosmetic state only, drawn at the
  same 25×40 size).
* `web/test_server.py`: web tests preserved for the first time (stdlib
  unittest, fake-socket WebSocket handler exercise, no network): green
  applies from Ship A's connection and appears in snapshots, g from Ship B
  / spectator is ignored, restart preserves the skin, a fresh server starts
  unskinned.

## [0.1.0] — 2026-09-29 arm64 revival

* Native arm64 (Apple Silicon) build against openFrameworks 0.12.1 — zero
  C++ source changes; only build config (`OF_ROOT` in config.make).
* `BUILD-arm64.md`: full build recipe (oF install at `~/lib`, make build,
  run options, controls, known issues — incl. the oF-makefile `/.local`
  path trap and `ofGetScreenHeight()==0` in non-GUI contexts).
* `web/`: rules-faithful 1v1 LAN remake (host-authoritative WebSocket
  server, 60 Hz sim transcribed from the C++, canvas client, original
  assets from `bin/data/`); rules covered by tests. `web/run.sh` prints
  the LAN URL.

## [Unversioned] Initial Commit

* Initial commit of the project.
* Added basic structure and files.
* Added LICENSE, Makefile, and README.md.
* Added initial implementation files (`src/`, `bin/`).

## [Unversioned] Initial Features & Milestones

* Added impulse drive and sounds for lazer, ship, shield, and explosion.
* Implemented basic GUI on the screen.
* Added basic green ship implementation.
* Implemented full screen support on smaller screens.
* Refactoring to support smaller screens (hide controls).
* Added gradle deployment support.
* Added basic gui on the screen.
* Added basic green ship.
* Added LICENSE and initial README.md updates.
* Added initial commit structure.

## [Unversioned] Enhancements

* Added basic gui on the screen.
* Added basic green ship.
* Added full screen support on smaller screens.
* Refactoring to support smaller screens (hide controls).
* Added gradle deployment support.
* Added impulse drive, and sounds for lazer, ship, shield, explosion, thrust.

## [Unversioned] Fixes

* Fixed issue addressed in commit #9.

---
*Note: The project history reflects several commits without formal version tags. The above summary groups related commits into logical milestones based on the commit messages.*
