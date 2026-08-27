"""
IoT console: simulation (no hardware) and live MQTT/serial bridge.
Run: python tools/gui/server.py


Copyright (c) 2026 Embedded AI Design Labs Pvt Ltd.
Embedded AI Labs — Intelligence at the Edge
Muhammad Samiullah — CTO & Founder
© 2026 Copyright. All rights reserved.
"""
from __future__ import annotations

import json
import os
import random
import threading
import time
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parent
STATIC = ROOT / "static"
STATE = {
    "mode": os.environ.get("IOT_MODE", "simulation"),
    "profile": "energy",
    "wifi": "WIFI_IP_READY" if os.environ.get("IOT_MODE") == "simulation" else "WIFI_DISCONNECTED",
    "mqtt": "IOT_MQTT_UP",
    "ble": "IOT_BLE_ADVERTISING",
    "seq": 0,
    "telemetry": {},
    "tasks": [
        {"name": "sensor", "prio": 12, "stack": 4096, "sync": "sensor_isr_sem"},
        {"name": "proc", "prio": 7, "stack": 4096, "sync": "sample_q"},
        {"name": "mqtt_pump", "prio": 8, "stack": 4096, "sync": "telemetry_q"},
        {"name": "cmd", "prio": 11, "stack": 4096, "sync": "command_q"},
        {"name": "diag", "prio": 3, "stack": 3072, "sync": "5s"},
        {"name": "wdt", "prio": 14, "stack": 2048, "sync": "esp_task_wdt_reset"},
    ],
}


def simulate_forever() -> None:
    while True:
        STATE["seq"] += 1
        p = STATE["profile"]
        if p == "energy":
            v, i = 230 + random.uniform(-2, 2), 0.4 + random.uniform(0, 1.2)
            STATE["telemetry"] = {
                "profile": p,
                "v": round(v, 1),
                "i": round(i, 3),
                "p": round(v * i, 1),
                "e_kwh": round(STATE["seq"] * 0.0002, 5),
                "alarm": int(v * i > 4000),
            }
        elif p == "water":
            STATE["telemetry"] = {
                "profile": p,
                "flow_lpm": round(random.uniform(0, 12), 2),
                "liters": round(STATE["seq"] * 0.02, 3),
                "tank_pct": round(random.uniform(40, 95), 1),
                "valve": 1,
                "leak": 0,
            }
        elif p == "industrial":
            STATE["telemetry"] = {
                "profile": p,
                "t": round(random.uniform(24, 78), 2),
                "vib": round(random.uniform(1, 45), 1),
                "state": "RUN",
            }
        else:
            STATE["telemetry"] = {"profile": p, "seq": STATE["seq"]}
        time.sleep(1.0)


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(STATIC), **kwargs)

    def log_message(self, fmt, *args):
        return

    def do_GET(self):
        if self.path.startswith("/api/state"):
            body = json.dumps(STATE).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(body)
            return
        if self.path == "/api/docs":
            # Redirect consumers to the static HTML portal.
            self.send_response(302)
            self.send_header("Location", "/index.html")
            self.end_headers()
            return
        return super().do_GET()

    def do_POST(self):
        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length) if length else b"{}"
        try:
            msg = json.loads(raw.decode() or "{}")
        except json.JSONDecodeError:
            msg = {}
        if self.path == "/api/mode":
            STATE["mode"] = msg.get("mode", STATE["mode"])
        elif self.path == "/api/profile":
            STATE["profile"] = msg.get("profile", STATE["profile"])
        body = json.dumps({"ok": True, "state": STATE}).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(body)


def main() -> None:
    threading.Thread(target=simulate_forever, daemon=True).start()
    port = int(os.environ.get("IOT_GUI_PORT", "8080"))
    httpd = ThreadingHTTPServer(("0.0.0.0", port), Handler)
    print(f"IoT GUI http://127.0.0.1:{port}  mode={STATE['mode']}")
    httpd.serve_forever()


if __name__ == "__main__":
    main()
