#!/usr/bin/env python3
"""Exercise an already-running WordClock without changing its settings or firmware."""

import argparse
import base64
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import os
from pathlib import Path
import socket
import struct
import time
import urllib.request


class DeviceCheck:
    def __init__(self, args):
        self.args = args
        self.base = "http://" + args.host
        self.http = urllib.request.build_opener(urllib.request.ProxyHandler({}))
        self.assets = Path(__file__).resolve().parent.parent / "data"
        self.last_uptime = None
        self.requests = 0
        self.last_report = 0

    def report(self, **data):
        line = json.dumps({"time": time.strftime("%Y-%m-%d %H:%M:%S"), **data})
        print(line, flush=True)
        if self.args.log:
            with open(self.args.log, "a") as out:
                out.write(line + "\n")

    def get(self, path):
        with self.http.open(self.base + path, timeout=8) as response:
            return response.read()

    def status(self):
        state = json.loads(self.get("/api/recovery/status"))
        self.requests += 1
        if state["mode"] != "normal" or state["sketch_md5"] != self.args.md5:
            raise RuntimeError(state)
        if self.last_uptime is not None and state["uptime_ms"] < self.last_uptime:
            raise RuntimeError("Device rebooted: " + repr(state))
        self.last_uptime = state["uptime_ms"]
        return state

    def pages(self, pool):
        paths = ("/", "/settings.html", "/settings.js", "/style.css", "/api/settings")
        bodies = list(pool.map(self.get, paths))
        self.requests += len(paths)
        for path, body in zip(paths[:-1], bodies[:-1]):
            name = "index.html" if path == "/" else path[1:]
            if body != (self.assets / name).read_bytes():
                raise RuntimeError("Incomplete or changed response: " + path)
        json.loads(bodies[-1])
        return self.status()

    def http_phase(self, seconds):
        until = time.monotonic() + seconds
        with ThreadPoolExecutor(max_workers=4) as pool:
            while time.monotonic() < until:
                state = self.pages(pool)
                if time.monotonic() - self.last_report >= 20:
                    self.report(phase="http", requests=self.requests, status=state)
                    self.last_report = time.monotonic()
                time.sleep(0.5)

    def abort_download(self, index):
        with socket.create_connection((self.args.host, 80), timeout=4) as client:
            client.sendall(
                ("GET /settings.js HTTP/1.1\r\nHost: " + self.args.host +
                 "\r\nConnection: close\r\n\r\n").encode()
            )
            if index % 3:
                client.recv(300 if index % 2 else 1200)
            client.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0))

    def websocket(self, index):
        with socket.create_connection((self.args.host, 80), timeout=4) as client:
            key = base64.b64encode(os.urandom(16))
            client.sendall(
                ("GET /ws HTTP/1.1\r\nHost: " + self.args.host +
                 "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: ").encode()
                + key + b"\r\nSec-WebSocket-Version: 13\r\n\r\n"
            )
            data = b""
            while b"\r\n\r\n" not in data:
                chunk = client.recv(4096)
                if not chunk:
                    raise RuntimeError("Incomplete WebSocket handshake")
                data += chunk
            headers, buffered = data.split(b"\r\n\r\n", 1)
            accept = base64.b64encode(hashlib.sha1(
                key + b"258EAFA5-E914-47DA-95CA-C5AB0DC85B11").digest())
            if not headers.startswith(b"HTTP/1.1 101") or accept not in headers:
                raise RuntimeError("Invalid WebSocket handshake: " + repr(headers))
            if index % 5 == 0:
                # Exercise split extended-length headers as well as close headers.
                payload = bytes(range(126))
                mask = os.urandom(4)
                frame = b"\x82\xfe\x00\x7e" + mask + bytes(
                    byte ^ mask[i % 4] for i, byte in enumerate(payload))
                for byte in frame[:8]:
                    client.sendall(bytes([byte]))
                    time.sleep(0.003)
                client.sendall(frame[8:])
            if index % 11 == 0:
                payload = b"ping"
                mask = os.urandom(4)
                frame = b"\x89\x84" + mask + bytes(
                    byte ^ mask[i % 4] for i, byte in enumerate(payload))
                for byte in frame:
                    client.sendall(bytes([byte]))
                    time.sleep(0.003)

                def read(size):
                    nonlocal buffered
                    while len(buffered) < size:
                        chunk = client.recv(4096)
                        if not chunk:
                            raise RuntimeError("WebSocket disconnected before pong")
                        buffered += chunk
                    result, buffered = buffered[:size], buffered[size:]
                    return result

                for _ in range(5):
                    first, length = read(2)
                    if length == 126:
                        length = struct.unpack("!H", read(2))[0]
                    elif length == 127:
                        length = struct.unpack("!Q", read(8))[0]
                    if length > 1024:
                        raise RuntimeError("Unexpected server frame length")
                    body = read(length)
                    if first & 0x0f == 10:
                        if body != b"ping":
                            raise RuntimeError("Corrupt pong payload")
                        break
                    if first & 0x0f != 1 or len(body) != 19:
                        raise RuntimeError("Invalid clock WebSocket frame")
                else:
                    raise RuntimeError("Missing pong response")
            if index % 3:
                mask = os.urandom(4)
                # The reason text has no trailing NUL on the wire.
                payload = b"\x03\xeaOK" if index % 7 == 0 else b"\x03\xe8"
                frame = b"\x88" + bytes([0x80 | len(payload)]) + mask + bytes(
                    byte ^ mask[i % 4] for i, byte in enumerate(payload))
                if index % 2:
                    for byte in frame:
                        client.sendall(bytes([byte]))
                        time.sleep(0.003)  # TCP can split a WebSocket frame anywhere.
                else:
                    client.sendall(frame)
            else:
                client.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0))

    def churn(self, label, operation, count):
        self.report(phase=label, state="starting", count=count)
        with ThreadPoolExecutor(max_workers=4) as pool:
            for start in range(0, count, 4):
                list(pool.map(operation, range(start, min(start + 4, count))))
                state = self.pages(pool)
                if start % 40 == 0:
                    self.report(phase=label, completed=min(start + 4, count), status=state)
                time.sleep(0.15)
        self.report(phase=label, state="passed", count=count)

    def run(self):
        settings = json.loads(self.get("/api/settings"))
        color = json.loads(self.get("/api/color"))
        appearance = self.get("/appearance.cfg")
        self.report(phase="begin", animation=settings["animation"], status=self.status())
        self.http_phase(self.args.seconds / 2)
        self.churn("aborted-downloads", self.abort_download, 100)
        self.churn("websocket-close", self.websocket, 240)
        self.http_phase(self.args.seconds / 2)
        state = self.status()
        if not state["application_healthy"]:
            raise RuntimeError("Application did not reach healthy state")
        after_settings = json.loads(self.get("/api/settings"))
        for key, value in settings.items():
            if key != "ambient_filtered_lux" and after_settings[key] != value:
                raise RuntimeError("Setting changed: " + key)
        after_color = json.loads(self.get("/api/color"))
        for key in ("color", "brightness"):
            if after_color[key] != color[key]:
                raise RuntimeError("Color setting changed: " + key)
        if self.get("/appearance.cfg") != appearance:
            raise RuntimeError("Persisted appearance changed")
        self.report(result="PASS", requests=self.requests, status=state)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", required=True)
    parser.add_argument("--md5", required=True, help="Expected running firmware MD5")
    parser.add_argument("--seconds", type=int, default=360, help="HTTP load duration, excluding connection churn")
    parser.add_argument("--log", help="Append JSON-lines results to this local file")
    args = parser.parse_args()
    if args.seconds < 75:
        parser.error("--seconds must be at least 75 to cross a real minute change")
    check = DeviceCheck(args)
    try:
        check.run()
    except Exception as error:
        check.report(result="FAIL", error=repr(error))
        # Capture reset details before anybody resumes/restarts the device.
        for _ in range(30):
            try:
                state = json.loads(check.get("/api/recovery/status"))
                check.report(phase="failure-status", status=state)
                break
            except (OSError, ValueError):
                time.sleep(1)
        raise


if __name__ == "__main__":
    main()
