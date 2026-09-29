/* Starships web 1v1 client — renderer + input. The server is authoritative.
   v0.1.2: hotseat/remote modes, `m` lazer-color menu, C++ Ship A mouse-button
   strip, OS key-repeat fire/shield, 1500x800 arena + 100px control strip.
   v0.1.4: `h` toggles the HUD legend (faded `h = help` hint stays when
   hidden; per-browser pref in localStorage). */
"use strict";

const AW = 1500, AH = 900;  // C++ window (Sizes.h MAX_WIDTH/MAX_HEIGHT)
const VB = 800;             // verticalBounds — ofApp.h controls = 100, the sim
                            // plays in 1500x800; we render the strip below it
const canvas = document.getElementById("game");
const hud = document.getElementById("hud");
const overlay = document.getElementById("overlay");

function fit() {
  // letterbox/scale the whole 1500x900 canvas (arena + strip) to the window
  const s = Math.min(window.innerWidth / AW, window.innerHeight / AH);
  canvas.style.width = (AW * s) + "px";
  canvas.style.height = (AH * s) + "px";
}
window.addEventListener("resize", fit);
fit();

/* ---- assets (served from the original game's bin/data) ---- */
const IMG = {};
for (const name of ["starfield-1500.jpg", "starship1.png", "starship_thrust.png",
                    "starship_purple.png", "starship_purple_thrust.png",
                    "bird_of_prey.png", "flame.png"]) {
  const im = new Image();
  im.src = "/assets/" + name;
  IMG[name] = im;
}
const SND = {};
for (const [key, file] of [
  ["fire", "SoundsCrate-SciFi-Laser2.mp3"],
  ["hit", "soundscrate-lightsword-turning-on-1.mp3"],
  ["explode", "SoundsCrate-SciFi-Explosion5.mp3"],
  ["thrust", "soundscrate-distant-rocket-exhaust-sc1.mp3"],
]) {
  SND[key] = new Audio("/assets/" + file);
  SND[key].preload = "auto";
}
SND.thrust.loop = true;
function play(name, vol) {
  if (name === "thrust") return; // handled as a loop
  try {
    const a = new Audio(SND[name].src);
    a.volume = vol; // C++ ofSoundPlayer default 1.0 (thrust loop 0.5)
    a.play().catch(() => {});
  } catch (e) { /* autoplay before user gesture — fine */ }
}

/* ---- networking ---- */
const wsPort = parseInt(location.port || "80", 10) + 1;
const ws = new WebSocket(`ws://${location.hostname}:${wsPort}`);
let me = null;          // "A" | "B" | null (spectator)
let mode = "remote";    // "hotseat" | "remote" — server-arbitrated
let state = null;       // latest server snapshot
let thrustWasOn = false;

function send(obj) { ws.send(JSON.stringify(obj)); }

ws.onopen = () => { overlay.classList.add("hidden"); };
ws.onclose = () => {
  overlay.classList.remove("hidden");
  overlay.textContent = "disconnected — reload the page to rejoin";
};
ws.onmessage = (ev) => {
  const msg = JSON.parse(ev.data);
  if (msg.t === "welcome") {
    me = msg.player;
    mode = msg.mode || "remote";
    if (me === "A" || me === "B") overlay.classList.add("hidden");
    else overlay.textContent = "spectating (two players already connected)";
  } else if (msg.t === "s") {
    state = msg;
    setMode(msg.mode);
    handleEvents(msg.e);
  }
};

function setMode(m) {
  if (m === mode || !m) return;
  mode = m;
  if (m === "remote") {
    // a second browser just claimed Ship B: I no longer fly it. Release
    // whatever B-keyset state I held so Ship B does not drift on its own.
    send({ t: "input", ship: "b", left: false, right: false, thrust: false });
    sendHeld("a"); // re-assert my own held keys under remote routing
  } else {
    // back to hotseat: re-assert both keysets so held keys keep working
    sendHeld("a");
    sendHeld("b");
  }
}

function handleEvents(events) {
  if (!events) return;
  for (const e of events) {
    if (e === "fire") play("fire", 1.0);
    else if (e === "hit") play("hit", 1.0);
    else if (e === "explode") play("explode", 1.0);
  }
}

/* ---- input (faithful keymaps) ----
   Ship A: a/d rotate, s thrust, w fire, x shield (+ g green skin).
   Ship B: numpad 4/6 rotate, 5 thrust, 8 fire, 2 shield OR l/' rotate,
   ; thrust, p fire, / shield (both sets live, as in ofApp). */
const KEYSET = {
  a: ["a", "left"],  d: ["a", "right"], s: ["a", "thrust"],
  "4": ["b", "left"], "6": ["b", "right"], "5": ["b", "thrust"],
  l: ["b", "left"], "'": ["b", "right"], ";": ["b", "thrust"],
};
const FIRE_KEYS = { w: "a", "8": "b", p: "b" };
const SHIELD_KEYS = { x: "a", "2": "b", "/": "b" };

// held-state per keyset (rotation/thrust are polled; fire/shield are edges)
const held = {
  a: { left: false, right: false, thrust: false },
  b: { left: false, right: false, thrust: false },
};
const btnHeld = { left: false, right: false, thrust: false };

function mergedHeld() {
  return {
    left: held.a.left || held.b.left,
    right: held.a.right || held.b.right,
    thrust: held.a.thrust || held.b.thrust,
  };
}
function sendHeld(set) {
  if (mode === "hotseat") send({ t: "input", ship: set, ...held[set] });
  else send({ t: "input", ...mergedHeld() }); // either keyset flies YOUR ship
}
function sendAction(action, set) {
  if (mode === "hotseat") send({ t: "input", ship: set, [action]: true });
  else send({ t: "input", [action]: true });
}

document.addEventListener("keydown", (ev) => {
  const k = ev.key.toLowerCase();
  const km = KEYSET[k];
  if (km) {
    const [set, act] = km;
    if (!held[set][act]) { held[set][act] = true; sendHeld(set); }
    return;
  }
  // C++ re-fires edge actions on OS key-repeat: HOLDING fire auto-fires,
  // HOLDING shield flickers on/off — no repeat guard here on purpose.
  if (FIRE_KEYS[k]) { sendAction("fire", FIRE_KEYS[k]); return; }
  if (SHIELD_KEYS[k]) { sendAction("shield", SHIELD_KEYS[k]); return; }
  if (ev.repeat) return; // one-shot guard: r/g/m/h must not machine-gun
  if (k === "r") send({ t: "input", restart: true });          // global
  else if (k === "g") send({ t: "input", ship: "a", green: true }); // Ship A only
  else if (k === "m") toggleMenu();
  else if (k === "h") { hudVisible = !hudVisible; applyHudPref(); }
});

/* ---- HUD legend visibility (`h` toggle, v0.1.4) ----
   Pure client toggle: hides the top-corner legend for clean gameplay and
   screenshots; a faded `h = help` hint stays in the same corner so the
   legend is always discoverable. Per-browser preference persisted in
   localStorage (default = shown); try/catch for sandboxed contexts. */
let hudVisible = true;
try { hudVisible = localStorage.getItem("starships-hud") !== "off"; } catch (e) {}
function applyHudPref() {
  hud.classList.toggle("dim", !hudVisible);
  try { localStorage.setItem("starships-hud", hudVisible ? "on" : "off"); } catch (e) {}
}
applyHudPref();
document.addEventListener("keyup", (ev) => {
  const km = KEYSET[ev.key.toLowerCase()];
  if (km) {
    const [set, act] = km;
    held[set][act] = false;
    sendHeld(set);
  }
});
window.addEventListener("blur", () => {
  for (const set of ["a", "b"]) {
    held[set].left = held[set].right = held[set].thrust = false;
  }
  btnHeld.left = btnHeld.right = btnHeld.thrust = false;
  if (mode === "hotseat") { sendHeld("a"); sendHeld("b"); }
  else sendHeld("a");
  sendBtn();
});

/* ---- Ship A mouse buttons (ofApp RectButtons, C++ semantics: they drive
   Ship A in ANY mode, from any client — sent with via:"button") ---- */
const BTN_SIZE = 19, BTN_BASE = "rgb(204,204,204)", BTN_HI = "rgb(153,153,153)";
const btnOffset = AW / 2 - 55; // ofApp::setup offset
const BUTTONS = {
  shoot:    { x: btnOffset + 40, y: VB + 10 },
  left:     { x: btnOffset + 20, y: VB + 30 },
  shield:   { x: btnOffset + 40, y: VB + 50 },
  right:    { x: btnOffset + 60, y: VB + 30 },
  thruster: { x: btnOffset + 40, y: VB + 30 },
};
const pointer = { x: -1, y: -1 }; // canvas coords, for hover highlight

function canvasPos(e) {
  const r = canvas.getBoundingClientRect();
  return {
    x: (e.clientX - r.left) * (canvas.width / r.width),
    y: (e.clientY - r.top) * (canvas.height / r.height),
  };
}
function overButton(b) {
  return pointer.x >= b.x && pointer.x <= b.x + BTN_SIZE &&
         pointer.y >= b.y && pointer.y <= b.y + BTN_SIZE;
}
function sendBtn() {
  send({ t: "input", ship: "a", via: "button", ...btnHeld });
}
canvas.addEventListener("pointermove", (e) => { Object.assign(pointer, canvasPos(e)); });
canvas.addEventListener("pointerdown", (e) => {
  const p = canvasPos(e);
  for (const [name, b] of Object.entries(BUTTONS)) {
    if (p.x < b.x || p.x > b.x + BTN_SIZE || p.y < b.y || p.y > b.y + BTN_SIZE) continue;
    if (name === "shoot") send({ t: "input", ship: "a", via: "button", fire: true });
    else if (name === "shield") send({ t: "input", ship: "a", via: "button", shield: true });
    else { btnHeld[name] = true; sendBtn(); } // left/right/thruster are held
  }
});
function releaseButtons() {
  if (btnHeld.left || btnHeld.right || btnHeld.thrust) {
    btnHeld.left = btnHeld.right = btnHeld.thrust = false;
    sendBtn();
  }
}
window.addEventListener("pointerup", releaseButtons);
canvas.addEventListener("pointerleave", () => {
  pointer.x = pointer.y = -1;
  releaseButtons();
});

/* ---- `m` lazer-color menu (C++ ofxPanel "menu": aLazer + bLazer) ----
   Mouse/touch only; the game keeps running and ALL keys still steer —
   the panel swallows no keyboard. */
const menuEl = document.getElementById("menu");
const colA = document.getElementById("colA");
const colB = document.getElementById("colB");

function rgbToHex(c) {
  return "#" + c.map(v => v.toString(16).padStart(2, "0")).join("");
}
function toggleMenu() {
  menuEl.classList.toggle("hidden");
  if (!menuEl.classList.contains("hidden") && state) {
    // initialize the pickers from the live server colors (shared panel)
    colA.value = rgbToHex(state.a.col);
    colB.value = rgbToHex(state.b.col);
  }
}
function sendColor(ship, hex) {
  const n = parseInt(hex.slice(1), 16);
  send({ t: "color", ship, r: (n >> 16) & 255, g: (n >> 8) & 255, b: n & 255 });
}
colA.addEventListener("input", () => sendColor("a", colA.value));
colB.addEventListener("input", () => sendColor("b", colB.value));

/* ---- rendering ---- */
const ctx = canvas.getContext("2d");
const SHIPS = {
  A: { body: "starship1.png", thrust: "starship_thrust.png" },
  B: { body: "starship_purple.png", thrust: "starship_purple_thrust.png" },
};
// green skin: BOTH frames load the same file — there is no green thrust art
// (faithful to the C++ 'g' handler, which loads SHIP_GREEN_IMAGE into both)
const GREEN_A = { body: "bird_of_prey.png", thrust: "bird_of_prey.png" };

function drawShip(s, files) {
  ctx.save();
  ctx.translate(s.x, s.y);
  ctx.rotate(s.r);
  // red "in trouble" ring (Ship::display, strength <= 1): drawn INSIDE the
  // rotated transform — the (0,-7) offset rotates with the ship
  if (s.active && s.hull <= 1) {
    ctx.strokeStyle = "rgb(255,0,0)";
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.ellipse(0, -7, 11.5, 11.5, 0, 0, Math.PI * 2);
    ctx.stroke();
  }
  const img = IMG[s.active ? (s.th ? files.thrust : files.body) : "flame.png"];
  if (img.complete && img.naturalWidth > 0) {
    ctx.drawImage(img, -12.5, -20, 25, 40);
  } else { // assets still loading: simple placeholder triangle
    ctx.fillStyle = s.th ? "#fa0" : "#ddd";
    ctx.beginPath(); ctx.moveTo(0, -20); ctx.lineTo(12, 20); ctx.lineTo(-12, 20);
    ctx.closePath(); ctx.fill();
  }
  ctx.restore();
  // shield (Shield::display): r=20 at (x, y+3), red when strength <= 1
  if (s.sh) {
    ctx.strokeStyle = s.shStr <= 1 ? "rgb(255,0,0)" : "rgb(255,255,255)";
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.ellipse(s.x, s.y + 3, 20, 20, 0, 0, Math.PI * 2);
    ctx.stroke();
  }
}

function drawButtons() {
  // control strip below verticalBounds: C++ ofBackground(255) shows white
  ctx.fillStyle = "#fff";
  ctx.fillRect(0, VB, AW, AH - VB);
  for (const b of Object.values(BUTTONS)) {
    ctx.fillStyle = overButton(b) ? BTN_HI : BTN_BASE;
    ctx.fillRect(b.x, b.y, BTN_SIZE, BTN_SIZE);
  }
}

function draw() {
  // arena (starfield spans the 1500x800 playfield only)
  const sf = IMG["starfield-1500.jpg"];
  if (sf.complete && sf.naturalWidth > 0) ctx.drawImage(sf, 0, 0, AW, VB);
  else { ctx.fillStyle = "#000"; ctx.fillRect(0, 0, AW, VB); }

  if (state) {
    // lazers (Lazer::display): 1px colored line, tip at endpoint; color is
    // read at draw time from the ship's shared color — recolors in flight
    ctx.lineWidth = 1;
    for (const l of state.L) {
      ctx.strokeStyle = `rgb(${l.c[0]},${l.c[1]},${l.c[2]})`;
      ctx.beginPath();
      ctx.moveTo(l.x, l.y);
      ctx.lineTo(l.x2, l.y2);
      ctx.stroke();
    }
    drawShip(state.a, state.a.skin === "green" ? GREEN_A : SHIPS.A);
    drawShip(state.b, SHIPS.B);
  }
  drawButtons();

  // HUD legend (hidden state shows the faded `h = help` hint instead —
  // the .dim class is applied by applyHudPref(), never fully invisible)
  const tag = mode === "hotseat"
    ? "HOTSEAT — one keyboard flies BOTH ships"
    : "remote 1v1";
  const hullTxt = state
    ? (mode === "hotseat" ? `A:${state.a.hull} B:${state.b.hull}`
                          : (me === "A" || me === "B") ? state[me.toLowerCase()].hull : "–")
    : "–";
  const legend =
    `You are Ship ${me || "?"}   [${tag}]   hull: ${hullTxt}\n` +
    (mode === "hotseat"
      ? "A: a/d s w x   B: 4/6 5 8 2 (or l/' ; p /)\n"
      : `rotate: ${me === "B" ? "4/6 or l/'" : "a/d"}   thrust: ${me === "B" ? "5 or ;" : "s"}   ` +
        `fire: ${me === "B" ? "8 or p" : "w"}   shield: ${me === "B" ? "2 or /" : "x"}\n`) +
    `r restart   m colors   h hud   ` + (me === "A" || mode === "hotseat" ? `g green skin (A)   ` : "") +
    `bottom buttons = Ship A mouse controls`;
  hud.textContent = hudVisible ? legend : "h = help";

  requestAnimationFrame(draw);
}
requestAnimationFrame(draw);

// own-thrust engine loop (thrustOn from server state)
setInterval(() => {
  const mine = state && me && state[me.toLowerCase()];
  const on = !!(mine && mine.th);
  if (on && !thrustWasOn) { SND.thrust.volume = 0.5; SND.thrust.play().catch(() => {}); }
  if (!on && thrustWasOn) { SND.thrust.pause(); SND.thrust.currentTime = 0; }
  thrustWasOn = on;
}, 100);
