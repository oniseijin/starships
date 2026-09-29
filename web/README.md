# Starships — web 1v1 (LAN)

A rules-faithful web remake of the native openFrameworks game (`src/*.cpp`):
same constants, same physics, same quirks. Two browsers on the LAN, one Mac
hosts. No accounts, no installs on the client side — just a browser.

## Try it on your LAN in 60 seconds

1. On the Mac that hosts (needs [uv](https://docs.astral.sh/uv/)):

   ```sh
   ~/workspace/Starships/web/run.sh
   ```

   It prints a LAN URL like `http://192.168.x.x:47777` (HTTP on 47777,
   WebSocket on 47778).

2. Open that URL in a browser — see **Modes** below for who flies what.
3. Fly, shoot, shield. `r` restarts (both ships), `m` opens the color menu,
   `h` hides the HUD legend (a faded `h = help` stays in the corner).

## Modes (server-arbitrated, mirrors the C++ one-keyboard couch setup)

- **Hotseat** — while only ONE browser is connected, it is the couch game:
  the A keyset flies Ship A, the B keysets fly Ship B, one keyboard (one
  window) driving both — exactly like the original.
- **Remote 1v1** — when a SECOND browser connects it claims Ship B; each
  client then controls only its own ship with either keyset. The HUD shows
  which mode you are in.
- **Takeover rule** — if Ship B's client disconnects, the remaining client
  returns to hotseat (and its B keyset drives Ship B again). A third+ browser
  spectates. The server arbitrates slot ownership; clients just render the
  indicator.

The Ship A mouse buttons (below) always drive Ship A in any mode, from any
client — the C++ mouse semantics.

## Controls

| action | keys |
|---|---|
| rotate | Ship A `a`/`d` — Ship B `4`/`6` or `l`/`'` |
| thrust | Ship A `s` — Ship B `5` or `;` |
| fire | Ship A `w` — Ship B `8` or `p` (holding auto-fires, like C++ key-repeat) |
| shield | Ship A `x` — Ship B `2` or `/` (holding flickers, like C++) |
| restart | `r` — global: resets BOTH ships (colors/skins survive) |
| green skin | `g` — Ship A only: switches to the green Bird-of-Prey (one-way, survives restart) |
| color menu | `m` — toggles the lazer-color panel (see below) |
| HUD toggle | `h` — hides the top-corner legend for clean gameplay/screenshots (hotseat + remote); a small faded `h = help` hint remains in the corner so the legend is always findable. Per-browser preference, survives reloads (localStorage; default = shown). |

In remote mode either keyset works for YOUR ship (both keysets are OR-ed,
as wired in `ofApp::keyPressed`).

## `m` — the lazer-color menu

Faithful port of the C++ ofxPanel: a small overlay panel with two color
pickers, **aLazer** (Ship A, default cyan `127,229,238`) and **bLazer**
(Ship B, default pink `255,192,204`), full 0–255 range per channel (draw
alpha is always 255, as in `Lazer::display`). Mouse/touch only — the game
KEEPS RUNNING while it is open and all keys still steer. Colors are
app-global (any connected client may set either; last write wins) and
survive `r`; a fresh server launch resets the defaults. The C++ quirk is
replicated: changing a color recolors ALREADY-IN-FLIGHT lazers instantly
(the server mutates one shared color per ship that every lazer references).

## Ship A mouse buttons

The C++ window reserves a 100px control strip below the arena with five
RectButtons for Ship A (shoot / left / right / thruster / shield, same
positions and gray highlight). The web client renders the same strip;
left/right/thruster are held buttons, shoot/shield are press-once. They
drive Ship A in ANY mode.

## Arena

The C++ game plays 1500 wide × (screenH − 100) tall — 1500×800 with a 100px
control strip on the reference 1500×900 window (spawn at 750,400; wrap
bounds 1500×800). The server can't know a browser's height, so the WORLD is
fixed at 1500×800 (spawn y=400) and the client letterboxes/scales a
1500×900 canvas (arena + strip) to fit any window.

## Rules (same as the original)

5 hull each (never recharges), shield absorbs hits but decays while up /
recharges while down, momentum physics with screen wrap, 10+1 live lazers
max per ship, self-hit grace of 20 frames, no win condition. Faithful
quirks kept: `lazers.size() <= MAX_LAZERS` off-by-one (11 max), lazer
lifetime 650 frames, ships spawn overlapping at centre, restart keeps
lazers/shield state (r is global — both ships reset), hull<=1 draws the red
ring INSIDE the ship's rotated transform, shield absorbs silently, lazers
draw 1px, OS key-repeat re-fires fire/shield. The green skin faithfully
mirrors the C++ `g` handler: there is no dedicated green thrust frame
(thrusting draws the identical bird_of_prey sprite) and there is no
switch-back — pressing `g` again just reloads the same texture. It survives
`r` (restart re-renders from the ship's base image) and only a fresh server
launch starts everyone unskinned.

## Architecture

- `server.py` — host-authoritative: runs the 60 Hz simulation (a direct
  Python transcription of `Ship::update`, `Lazer::update`, `Shield::update`,
  and `ofApp::handleControls` ordering), broadcasts snapshots at 30 Hz,
  routes client inputs (hotseat/remote arbitration, per-keyset tags, Ship A
  button messages, menu color-set actions). Serves `static/` + the original
  art/sounds from `bin/data/` at `/assets/`. Stdlib HTTP + `websockets`
  (the only dependency, resolved by uv). Binds `0.0.0.0`.
- `static/client.js` — canvas renderer (arena letterboxing, control strip,
  m-menu overlay) + keyboard/mouse input, no dependencies.
- `test_server.py` — stdlib unittest suite (fake-socket WebSocket handler
  exercise, no network): green skin, color menu, repeat-fire, hotseat
  arbitration, restart semantics.

Why not an Emscripten port of the oF app? It would still have no networking
(the original is two players on one keyboard), and every rule needed for a
remake is ~700 lines of simple logic — see the verdict notes in
`BUILD-arm64.md`.
