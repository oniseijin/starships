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

2. Open that URL in a browser on the host Mac — you are **Ship A** (cyan).
3. Open the same URL on any other device on the same LAN — that one is
   **Ship B** (purple). Anyone else who opens it spectates.
4. Fly, shoot, shield. `r` restarts. Have at it.

## Controls (either keyset works for YOUR ship)

| action | keys |
|---|---|
| rotate | `a`/`d` or `4`/`6` or `l`/`'` |
| thrust | `s` or `5` or `;` |
| fire | `w` or `8` or `p` |
| shield | `x` or `2` or `/` |
| restart | `r` |
| green skin | `g` — Ship A only: switches to the green Bird-of-Prey (one-way, survives restart) |

Same rules as the original: 5 hull each (never recharges), shield absorbs
hits but decays while up / recharges while down, momentum physics with
screen wrap, 10+1 live lazers max per ship, self-hit grace of 20 frames.

## Architecture

- `server.py` — host-authoritative: runs the 60 Hz simulation (a direct
  Python transcription of `Ship::update`, `Lazer::update`, `Shield::update`,
  and `ofApp::handleControls` ordering), broadcasts snapshots at 30 Hz,
  applies client input flags. Serves `static/` + the original art/sounds
  from `bin/data/` at `/assets/`. Stdlib HTTP + `websockets` (the only
  dependency, resolved by uv). Binds `0.0.0.0`.
- `static/client.js` — canvas renderer + keyboard input, no dependencies.

Faithful quirks kept: `lazers.size() <= MAX_LAZERS` off-by-one (11 max),
lazer lifetime 650 frames, ships spawn overlapping at centre, restart keeps
lazers/shield state, hull<=1 draws the red ring, shield absorbs silently.
The green skin faithfully mirrors the C++ `g` handler: there is no dedicated
green thrust frame (thrusting draws the identical bird_of_prey sprite) and
there is no switch-back — pressing `g` again just reloads the same texture.
It survives `r` (restart re-renders from the ship's base image) and only a
fresh server launch starts everyone unskinned.

Why not an Emscripten port of the oF app? It would still have no networking
(the original is two players on one keyboard), and every rule needed for a
remake is ~700 lines of simple logic — see the verdict notes in
`BUILD-arm64.md`.
