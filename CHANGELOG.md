# CHANGELOG

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
