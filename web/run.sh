#!/bin/bash
# Starships web 1v1 — start the LAN server (port 47777, ws on 47778).
# Requires uv (https://docs.astral.sh/uv/): Python 3.13 + websockets are
# resolved automatically on first run.
cd "$(dirname "$0")/.." || exit 1
exec uv run --python 3.13 --with websockets web/server.py "${1:-47777}"
