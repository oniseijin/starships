#!/usr/bin/env python3
"""Regenerate Starships.icns with size-appropriate art.

Two variants live in ONE .icns (macOS picks the right size):

* "duel" (detailed) — the two-ship duel scene with crossing pink/cyan
  lazers, used for 256-1024 px where the detail reads (Finder icon view
  at large sizes, DMG, App Store style previews).
* "solo" (simple)  — ONE ship, big, on a dark starfield with a single
  cyan lazer streak; used for 16-128 px where the duel collapses into an
  unrecognizable smear ("mop with a purple brush").

Outputs a .iconset (both variants mapped to the standard 10 entries) and
converts it with `iconutil -c icns`. Renders each variant once at master
size (256 solo / 1024 duel) and downscales with LANCZOS for consistent
anti-aliasing.

Usage (from the repo root):
    uv run --with pillow tools/make_icon.py            # rebuild Starships.icns
    uv run --with pillow tools/make_icon.py --preview  # + side-by-side previews in /tmp

Pipeline after the icns changes: `make after` (embeds it into
bin/Starships.app as Resources/of.icns), then redeploy the .app.
"""

import argparse
import math
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageEnhance

REPO = Path(__file__).resolve().parent.parent
ASSETS = REPO / "bin" / "data"
DEFAULT_OUT = REPO / "Starships.icns"

# --- palette / geometry sampled from the original duel icon -----------------
PINK = (252, 178, 192)
CYAN = (120, 235, 250)
MASK_RADIUS = 0.215          # full-bleed rounded-rect, measured off the old icns
STARFIELD_CROP = (350, 0, 1150, 800)  # 800x800 square from starfield-1500.jpg

# duel layout, fractions of canvas (measured off the original 512 art)
DUEL = {
    "white":  {"center": (0.26, 0.28), "height": 0.36, "angle": -22},
    "purple": {"center": (0.76, 0.75), "height": 0.34, "angle": 200},
    "pink":   {"from": (0.33, 0.19), "to": (0.70, 0.76), "width": 0.042},
    "cyan":   {"from": (0.37, 0.38), "to": (0.79, 0.64), "width": 0.034},
}
# solo layout — silhouette first: ship ~72% of canvas height, upright;
# the single lazer passes behind the nacelles so the saucer stays clean
SOLO = {
    "ship":   {"center": (0.50, 0.48), "height": 0.72, "angle": 0},
    "streak": {"from": (0.16, 0.90), "to": (0.84, 0.40), "width": 0.032},
    "bg_gain": 0.40,           # starfield darkened hard: stars must not turn to noise
    "bg_blur": 1.2,            # soften star noise further at the master render
}

ICONSET_MAP = [
    # (iconset filename, variant, rendered size)
    ("icon_16x16.png",     "solo", 16),
    ("icon_16x16@2x.png",  "solo", 32),
    ("icon_32x32.png",     "solo", 32),
    ("icon_32x32@2x.png",  "solo", 64),
    ("icon_128x128.png",   "solo", 128),
    ("icon_128x128@2x.png", "duel", 256),
    ("icon_256x256.png",   "duel", 256),
    ("icon_256x256@2x.png", "duel", 512),
    ("icon_512x512.png",   "duel", 512),
    ("icon_512x512@2x.png", "duel", 1024),
]


starfield_img = None
ships = {}


def load_assets():
    global starfield_img, ships
    starfield_img = Image.open(ASSETS / "starfield-1500.jpg").convert("RGB")
    for name in ("starship1", "starship_purple"):
        im = Image.open(ASSETS / f"{name}.png").convert("RGBA")
        im = im.crop(im.getchannel("A").getbbox())  # trim transparent padding
        ships[name] = im


def rounded_mask(size):
    mask = Image.new("L", (size, size), 0)
    d = ImageDraw.Draw(mask)
    r = round(size * MASK_RADIUS)
    d.rounded_rectangle([0, 0, size - 1, size - 1], radius=r, fill=255)
    return mask


def starfield_bg(size, gain=1.0, blur=0):
    bg = starfield_img.crop(STARFIELD_CROP).resize((size, size), Image.LANCZOS)
    if blur:
        bg = bg.filter(ImageFilter.GaussianBlur(blur))
    if gain != 1.0:
        bg = Image.eval(bg, lambda v: int(v * gain))
    return bg.convert("RGBA")


def paste_ship(canvas, ship, spec):
    size = canvas.size[0]
    h = int(spec["height"] * size)
    w = int(h * ship.size[0] / ship.size[1])
    im = ship.resize((w, h), Image.LANCZOS)
    if spec["angle"]:
        im = im.rotate(spec["angle"], resample=Image.BICUBIC, expand=True)
    cx, cy = spec["center"][0] * size, spec["center"][1] * size
    canvas.alpha_composite(im, (int(cx - im.size[0] / 2), int(cy - im.size[1] / 2)))


def draw_streak(canvas, spec, color):
    """Lazer streak: soft wide glow + solid core, on its own layer."""
    size = canvas.size[0]
    p1 = (spec["from"][0] * size, spec["from"][1] * size)
    p2 = (spec["to"][0] * size, spec["to"][1] * size)
    core_w = max(2, round(spec["width"] * size))

    layer = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    d.line([p1, p2], fill=color + (255,), width=core_w)
    # one blurred pass under the core gives a soft glow (double pass = too neon)
    glow = layer.filter(ImageFilter.GaussianBlur(core_w * 0.8))
    canvas.alpha_composite(glow)
    canvas.alpha_composite(layer)


def render_duel(size):
    canvas = starfield_bg(size)
    paste_ship(canvas, ships["starship1"], DUEL["white"])
    paste_ship(canvas, ships["starship_purple"], DUEL["purple"])
    draw_streak(canvas, DUEL["pink"], PINK)
    draw_streak(canvas, DUEL["cyan"], CYAN)
    canvas.putalpha(rounded_mask(size))
    return canvas


def render_solo(size):
    canvas = starfield_bg(size, gain=SOLO["bg_gain"], blur=SOLO["bg_blur"])
    draw_streak(canvas, SOLO["streak"], CYAN)
    ship = ships["starship1"]
    # silhouette pop: brighten + contrast so the hull cuts out of the dark bg
    ship = ImageEnhance.Brightness(ship).enhance(1.15)
    ship = ImageEnhance.Contrast(ship).enhance(1.20)
    paste_ship(canvas, ship, SOLO["ship"])
    canvas.putalpha(rounded_mask(size))
    return canvas


def render_master(variant):
    return render_solo(256) if variant == "solo" else render_duel(1024)


def downscale(master, size):
    if master.size[0] == size:
        return master.copy()
    return master.resize((size, size), Image.LANCZOS)


def build_iconset(outdir):
    masters = {v: render_master(v) for v in ("solo", "duel")}
    outdir.mkdir(parents=True, exist_ok=True)
    for name, variant, size in ICONSET_MAP:
        downscale(masters[variant], size).save(outdir / name)
    return masters


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT, help="output .icns path")
    ap.add_argument("--preview", action="store_true",
                    help="also write a side-by-side preview grid to /tmp/starships-icon-preview.png")
    args = ap.parse_args()

    load_assets()
    with tempfile.TemporaryDirectory() as tmp:
        iconset = Path(tmp) / "Starships.iconset"
        masters = build_iconset(iconset)
        subprocess.run(["iconutil", "-c", "icns", str(iconset), "-o", str(args.out)],
                       check=True)
        print(f"wrote {args.out} ({args.out.stat().st_size} bytes)")

    if args.preview:
        sizes = [1024, 512, 256, 128, 64, 32, 16]
        pad, label_h = 8, 22
        W = sum(s + pad for s in sizes) + pad
        H = max(sizes) + label_h + 2 * pad
        grid = Image.new("RGB", (W, H), (40, 40, 48))
        d = ImageDraw.Draw(grid)
        x = pad
        for s in sizes:
            icon = downscale(masters["duel"] if s >= 256 else masters["solo"], s)
            grid.paste(icon, (x, pad), icon)
            d.text((x, pad + max(sizes) + 4), f"{s}px", fill=(220, 220, 220))
            x += s + pad
        out = Path("/tmp/starships-icon-preview.png")
        grid.save(out)
        print(f"preview: {out}")


if __name__ == "__main__":
    sys.exit(main())
