async function post(path, obj) {
  await fetch(path, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(obj),
  });
}

async function tick() {
  const res = await fetch("/api/state");
  const s = await res.json();
  document.getElementById("telemetry").textContent = JSON.stringify(s.telemetry, null, 2);
  document.getElementById("conn").innerHTML =
    `<li>Mode: <b>${s.mode}</b></li>` +
    `<li>Wi-Fi: ${s.wifi}</li>` +
    `<li>MQTT: ${s.mqtt}</li>` +
    `<li>BLE: ${s.ble}</li>` +
    `<li>Seq: ${s.seq}</li>`;
  document.getElementById("task-body").innerHTML = s.tasks
    .map((t) => `<tr><td>${t.name}</td><td>${t.prio}</td><td>${t.stack}</td><td>${t.sync}</td></tr>`)
    .join("");
}

document.getElementById("mode").addEventListener("change", (e) => {
  post("/api/mode", { mode: e.target.value });
});
document.getElementById("profile").addEventListener("change", (e) => {
  post("/api/profile", { profile: e.target.value });
});

setInterval(tick, 1000);
tick();
