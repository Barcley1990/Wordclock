"""Apply the reviewed, version-checked fix for split WebSocket frame headers."""

import hashlib
import json
from pathlib import Path
import subprocess

from SCons.Script import Import

Import("env")

project = Path(env.subst("$PROJECT_DIR"))
library = Path(env.subst("$PROJECT_LIBDEPS_DIR")) / env["PIOENV"] / "ESPAsyncWebServer"
if json.loads((library / "library.json").read_text())["version"] != "3.12.1":
    raise RuntimeError("Review the WebSocket frame patch before changing ESPAsyncWebServer")

# Refuse changed upstream sources rather than silently building an unsafe parser.
original = {
    "AsyncWebSocket.h": "d769838f7502f1269572290f35ee543cc2ae8ea7a78c1b07a9859530071db783",
    "AsyncWebSocket.cpp": "7d44f6c010db6394bea728d708fb0a06ede222c57c037f25ee06a1d9a277c1bd",
}
patched = {
    "AsyncWebSocket.h": "aac5b9e85111e5d52c564134e1dac2fd8445736aa46f95f49fc056a1f8cc43f0",
    "AsyncWebSocket.cpp": "dee90d5cd34df2ede3b90f848db00dd8e3ffb13bd5afb4b03e4d0f14abeb6138",
}


def hashes():
    return {name: hashlib.sha256((library / "src" / name).read_bytes()).hexdigest() for name in original}


current = hashes()
if current == original:
    subprocess.run([
        "git", "apply", "--unsafe-paths", "--directory", str(library),
        str(project / "patches" / "ESPAsyncWebServer-3.12.1-frame-header.patch"),
    ], cwd=project, check=True)
elif current != patched:
    raise RuntimeError("Unexpected ESPAsyncWebServer sources; refusing to skip the frame-header fix")

if hashes() != patched:
    raise RuntimeError("WebSocket patch verification failed")

helper = project / "patches" / "WebSocketFrameHeader.h"
target = library / "src" / helper.name
content = helper.read_bytes()
if not target.exists() or target.read_bytes() != content:
    target.write_bytes(content)
