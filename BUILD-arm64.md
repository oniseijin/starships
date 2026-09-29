# Building Starships on arm64 Apple Silicon (2026 revival)

The project was built 2014–2020 with openFrameworks 0.10.1/0.11.0 on x86_64
macOS. oF added proper Apple Silicon support in **0.12.x**; this repo now
builds native arm64 against **openFrameworks 0.12.1** with **zero C++ source
changes** (the 2026 revival changed only build config).

## Requirements

- macOS on Apple Silicon, Xcode Command Line Tools (`xcode-select --install`)
  — full Xcode is NOT required for the make build (plain `clang` + SDK).
- openFrameworks **0.12.1** (osx release) installed at `~/lib/of_v0.12.1_osx_release`.

## One-time: install openFrameworks 0.12.1

```sh
mkdir -p ~/lib && cd ~/lib
curl -LO https://github.com/openframeworks/openFrameworks/releases/download/0.12.1/of_v0.12.1_osx_release.tar.gz
tar xzf of_v0.12.1_osx_release.tar.gz && rm of_v0.12.1_osx_release.tar.gz
```

**Why `~/lib` and not `~/.local/lib`:** the oF makefiles discover core
sources with a "skip hidden dirs" filter — `find … | grep -v "/\.[^\.]"`.
Every path under `~/.local` contains the component `/.local`, so the filter
matches it and **excludes ALL core sources** (the build then fails with
`find: illegal option -- n` and an empty `ar` archive). Keep oF under a path
with no `/.` component. (`config.make` sets `OF_ROOT = $(HOME)/lib/of_v0.12.1_osx_release`.)

## Build

```sh
cd ~/workspace/Starships
make            # Release (default), builds oF core once on first run
# make Debug    # debug build if wanted
```

First build compiles the oF core library (~2–3 min); afterwards only project
sources rebuild. Third-party headers (utf8cpp etc.) emit warnings — harmless.

## Run

```sh
open bin/Starships.app
# or:
make RunRelease
# or directly:
bin/Starships.app/Contents/MacOS/Starships
```

## How to play

Local 2-player duel (Asteroids-style: rotate, thrust, shoot, shield;
5 hull each, hull never recharges; shield decays while up, recharges off;
`r` restarts; `m` toggles the lazer-color menu).

| Action | Ship A (cyan lazer) | Ship B (purple ship, numpad) | Ship B (alt) |
|---|---|---|---|
| rotate L/R | `a` / `d` | `4` / `6` | `l` / `'` |
| thrust | `s` | `5` | `;` |
| shoot | `w` | `8` | `p` |
| shield | `x` | `2` | `/` |

## Known issues / notes

- `ofGetScreenHeight()/Width()` can return 0 when the app is launched from a
  non-GUI (SSH/agent) context — the game still runs but sizes itself wrong.
  Launch from Terminal.app or Finder (windowed, max 1500×900; controls hide
  if the screen is shorter than 900).
- Verified build: Apple clang 21 (Xcode CLT), macOS 26.x, arm64, oF 0.12.1
  Release — 2026-09-29. Debug target also compiles.
- The legacy `Starships.xcodeproj` and `build.gradle` targets were NOT
  updated; the make build is the supported path (same as 2020).
