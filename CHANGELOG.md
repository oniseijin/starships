# CHANGELOG

## [Unreleased]

* Icon: size-appropriate art in ONE .icns. 16-128 px now use a "solo"
  variant — one ship (starship1), upright, ~72% of canvas height on a
  darkened starfield with a single cyan lazer streak behind the nacelles
  — while 256-1024 px keep the detailed duel scene. At Dock/Finder small
  sizes the old all-sizes duel collapsed into a gray/purple smear ("mop
  with a purple brush"); the solo variant reads as a spaceship at 32 px.
  Regenerable: `uv run --with pillow tools/make_icon.py` composes both
  variants from `bin/data` assets (Pillow, master renders + LANCZOS
  downscale, rounded-rect mask measured off the original art) and runs
  `iconutil -c icns`. `--preview` writes a size grid to /tmp.

## [0.1.7] - 2026-10-01

* Windows x64 release builds — the release workflow now also builds a
  Win64 exe on every `v*` tag and attaches `Starships-<tag>-win64.zip` to
  the same release as the mac artifacts (3 files total). Recipe: a
  `windows-latest` job under msys2/setup-msys2 (MSYSTEM=MINGW64, the
  oF-recommended flavor), openFrameworks 0.12.1
  `of_v0.12.1_msys2_mingw64_release.zip` cached extracted at
  `C:\of_v0.12.1_msys2_mingw64_release` (incl. the compiled core, same
  pattern as the mac job), toolchain + oF libs via the oF-shipped
  `scripts/msys2/install_dependencies.sh` (unzip/make + gcc + assimp
  cairo curl FreeImage glew glfw glm fmt zlib brotli libpng harfbuzz
  libsndfile libusb libxml2 mpg123 nlohmann-json openal opencv pkgconf
  pugixml rtaudio uriparser utf8cpp — re-run every run since pacman state
  isn't cached) plus `mingw-w64-x86_64-ntldd-git` for `make copy_dlls`.
  Package = `Starships.exe` + mingw runtime DLLs (copied next to the exe
  by `copy_dlls`) + `data/` (oF's default Windows data path) →
  Compress-Archive → upload-artifact → one final `release` job attaches
  everything with a single softprops call (no two-job release race).
* `config.make`: OF_ROOT is now OS-conditional — `uname` MINGW*/MSYS*
  (MSYS2 on Windows) points at `/c/of_v0.12.1_msys2_mingw64_release`,
  everything else keeps `$(HOME)/lib/of_v0.12.1_osx_release`. Zero change
  for the mac build (same resolved path as before).
* `Makefile`: the `after` target (bundle data/icon embed + plutil) is
  now guarded `ifeq ($(shell uname -s),Darwin)` — mac behavior unchanged
  (warning about overriding oF's own `after` predates this), and on
  Windows it can never fire.
* `main.cpp` needs no change: the `TARGET_OSX` Resources/data override is
  compiled only on macOS, so Windows uses oF's default exe-adjacent
  `data/`. Audio works on Windows out of the box (rtAudio ships via the
  oF msys2 dependency list).

## [0.1.6] - 2026-10-02

- Release builds now ship a DMG (drag-to-Applications) alongside the zip.
- App icon fix: CFBundleIconFile substituted (oF template left literal
  ${ICON}) - the duel icon now displays; after-hook makes it permanent.


## [0.1.5] — 2026-09-30

* CI/CD: GitHub Actions release path for the native arm64 app —
  `.github/workflows/release.yml` builds `Starships.app` on a macOS arm64
  runner on every `v*` tag push (openFrameworks 0.12.1 installed with the
  exact BUILD-arm64.md recipe at `~/lib`, which `config.make`'s `OF_ROOT`
  already points at; `make -j` + the `make after` data/icon embed), zips it
  (`Starships-<tag>-arm64.zip`) and attaches it to the GitHub Release
  (release body documents the unsigned-build Gatekeeper step). The ~1 GB oF
  download + core-library build are cached with `actions/cache` (keyed on
  the oF version; the cache keeps the extracted tree INCLUDING the compiled
  core, so warm runs skip both). `.github/workflows/ci.yml` runs the same
  build on every push to master / PR as a compile sanity check and
  smoke-checks the bundle (executable + embedded data + icon present).
* `web/`: touch controls — on coarse-pointer (touch) clients, on-screen
  thumb pads appear in the bottom corners (56 px targets) once a ship is
  assigned: Ship A's client gets the v0.1.2 C++ RectButton strip extended
  to thumb size (`◀ / THR / ▶` held, `FIRE` / `SHLD` press-once — the SAME
  `via:"button"` messages, so they drive Ship A from any client in any
  mode); Ship B's client (B has no strip in the C++ window) gets its own
  `◀ / THR / ▶` + `FIRE` / `SHLD` set on the B keyset semantics; both get
  an `RST` pad (restart, since `r` needs a keyboard). Pads are independent
  buttons with pointer capture: multi-touch works (thrust + steer
  concurrently), held pads light up, and backgrounding the browser
  releases everything (mirroring the keyboard blur handler). Client-only —
  `server.py` and the wire protocol untouched; desktop clients render
  nothing (gate = `pointer: coarse`). Verification: a DOM-level harness
  drove the real `client.js` with a stubbed DOM (14 checks: pad layout,
  exact wire frames per pad, multi-touch hold, blur release, desktop
  renders nothing) plus a live replay of the pad frames against a
  throwaway server instance — thrust/fire/restart visible in snapshots;
  `test_server.py` green untouched (34 tests).

## [0.1.4] — 2026-09-30

* `web/`: `h` toggles the HUD legend (one-shot, repeat-guarded like
  `m`/`g`/`r`; works in hotseat and remote; `h` is unused by any ship, so
  game input is untouched). When hidden, the same corner keeps a small
  faded `h = help` hint (opacity 0.35, 11px) so the legend is always
  discoverable — never fully invisible. The legend text now lists the key
  (`r restart   m colors   h hud …`). Per-browser preference persisted in
  localStorage (default = shown). Pure client toggle — no server or
  wire-protocol changes (`server.py` untouched; test suite green).

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
