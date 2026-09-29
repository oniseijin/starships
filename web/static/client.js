/* Starships web 1v1 client — renderer + input. The server is authoritative. */
"use strict";

const W = 1500, H = 900;
const canvas = document.getElementById("game");
const hud = document.getElementById("hud");
const overlay = document.getElementById("overlay");

function fit() {
  const s = Math.min(window.innerWidth / W, window.innerHeight / H);
  canvas.style.width = (W * s) + "px";
  canvas.style.height = (H * s) + "px";
}
window.addEventListener("resize", fit);
fit();

/* ---- assets (served from the original game's bin/data) ---- */
const IMG = {};
for (const name of ["starfield-1500.jpg", "starship1.png", "starship_thrust.png",
                    "starship_purple.png", "starship_purple_thrust.png", "flame.png"]) {
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
    a.volume = vol || 0.4;
    a.play().catch(() => {});
  } catch (e) { /* autoplay before user gesture — fine */ }
}

/* ---- networking ---- */
const wsPort = parseInt(location.port || "80", 10) + 1;
const ws = new WebSocket(`ws://${location.hostname}:${wsPort}`);
let me = null;          // "A" | "B" | null (spectator)
let state = null;       // latest server snapshot
let thrustWasOn = false;

ws.onopen = () => { overlay.classList.add("hidden"); };
ws.onclose = () => {
  overlay.classList.remove("hidden");
  overlay.textContent = "disconnected — reload the page to rejoin";
};
ws.onmessage = (ev) => {
  const msg = JSON.parse(ev.data);
  if (msg.t === "welcome") {
    me = msg.player;
    if (me === "A" || me === "B") overlay.classList.add("hidden");
    else overlay.textContent = "spectating (two players already connected)";
  } else if (msg.t === "s") {
    state = msg;
    handleEvents(msg.e);
  }
};

function handleEvents(events) {
  if (!events) return;
  for (const e of events) {
    if (e === "fire") play("fire", 0.35);
    else if (e === "hit") play("hit", 0.5);
    else if (e === "explode") play("explode", 0.7);
  }
}

/* ---- input (faithful keymaps; either keyset controls YOUR ship) ---- */
const input = { left: false, right: false, thrust: false };
const KEYMAP = {
  a: "left", 4: "left", l: "left",
  d: "right", 6: "right", "'": "right",
  s: "thrust", 5: "thrust", ";": "thrust",
};
const FIRE_KEYS = new Set(["w", "8", "p"]);
const SHIELD_KEYS = new Set(["x", "2", "/"]);

function sendInput(extra) {
  ws.send(JSON.stringify(Object.assign({ t: "input" }, input, extra || {})));
}
document.addEventListener("keydown", (ev) => {
  if (ev.repeat) return;
  const k = ev.key.toLowerCase();
  if (KEYMAP[k]) { input[KEYMAP[k]] = true; sendInput(); }
  else if (FIRE_KEYS.has(k)) sendInput({ fire: true });
  else if (SHIELD_KEYS.has(k)) sendInput({ shield: true });
  else if (k === "r") sendInput({ restart: true });
});
document.addEventListener("keyup", (ev) => {
  const k = ev.key.toLowerCase();
  if (KEYMAP[k]) { input[KEYMAP[k]] = false; sendInput(); }
});
window.addEventListener("blur", () => {
  input.left = input.right = input.thrust = false;
  sendInput();
});

/* ---- rendering ---- */
const ctx = canvas.getContext("2d");
const SHIPS = {
  A: { body: "starship1.png", thrust: "starship_thrust.png" },
  B: { body: "starship_purple.png", thrust: "starship_purple_thrust.png" },
};

function drawShip(s, files) {
  // red "in trouble" halo (Ship::display, strength <= 1)
  if (s.active && s.hull <= 1) {
    ctx.strokeStyle = "rgb(255,0,0)";
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.ellipse(s.x, s.y - 7, 11.5, 11.5, 0, 0, Math.PI * 2);
    ctx.stroke();
  }
  ctx.save();
  ctx.translate(s.x, s.y);
  ctx.rotate(s.r);
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

function draw() {
  const sf = IMG["starfield-1500.jpg"];
  if (sf.complete && sf.naturalWidth > 0) ctx.drawImage(sf, 0, 0, W, H);
  else { ctx.fillStyle = "#000"; ctx.fillRect(0, 0, W, H); }

  if (state) {
    // lazers (Lazer::display): colored line, tip at endpoint
    ctx.lineWidth = 3;
    for (const l of state.L) {
      ctx.strokeStyle = `rgb(${l.c[0]},${l.c[1]},${l.c[2]})`;
      ctx.beginPath();
      ctx.moveTo(l.x, l.y);
      ctx.lineTo(l.x2, l.y2);
      ctx.stroke();
    }
    drawShip(state.a, SHIPS.A);
    drawShip(state.b, SHIPS.B);
  }

  // HUD
  const mine = state && me !== "A" && me !== "B" ? null : state && state[me.toLowerCase()];
  const hull = mine ? mine.hull : "–";
  hud.textContent =
    `You are Ship ${me || "?"}   hull: ${hull}\n` +
    `rotate: ${me === "B" ? "4/6 or l/'" : "a/d"}   thrust: ${me === "B" ? "5 or ;" : "s"}   ` +
    `fire: ${me === "B" ? "8 or p" : "w"}   shield: ${me === "B" ? "2 or /" : "x"}   restart: r`;

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
