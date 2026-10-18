"""Build the ESP8266-compatible LittleFS image tool natively on Apple Silicon."""

from pathlib import Path
import platform
import subprocess

from SCons.Script import COMMAND_LINE_TARGETS, Import

Import("env")

# Newer prebuilt mklittlefs releases use a different disk format and name limit.
# 3.2.0 uses LittleFS 2.5.1 and LFS_NAME_MAX=32, matching Arduino ESP8266 3.1.2.
REVISION = "975bd0f28e0976c1c018062537c23c8768b09076"
LITTLEFS_REVISION = "6a53d76e90af33f0656333c1db09bd337fa75d23"


def configure_native_mklittlefs():
    if not set(COMMAND_LINE_TARGETS) & {"buildfs", "uploadfs", "uploadfsota"}:
        return
    if platform.system() != "Darwin" or platform.machine() != "arm64":
        raise RuntimeError("Use esp_wroom_02_macos_arm64 only on an Apple Silicon Mac")

    source = Path(env.subst("$PROJECT_DIR")) / ".pio" / "tools" / ("mklittlefs-" + REVISION)
    tool = source / "mklittlefs"
    if not tool.is_file():
        source.parent.mkdir(parents=True, exist_ok=True)
        if not source.exists():
            subprocess.run([
                "git", "clone", "--recursive", "--depth", "1", "--branch", "3.2.0",
                "https://github.com/earlephilhower/mklittlefs.git", str(source),
            ], check=True)
        for directory, revision in ((source, REVISION), (source / "littlefs", LITTLEFS_REVISION)):
            actual = subprocess.check_output(
                ["git", "-C", str(directory), "rev-parse", "HEAD"], text=True
            ).strip()
            if actual != revision:
                raise RuntimeError("Unexpected mklittlefs source revision in " + str(directory))
        subprocess.run([
            "make", "-C", str(source), "-j2", "CC=/usr/bin/clang", "CXX=/usr/bin/clang++",
            "CXXFLAGS=-Wno-error", "VERSION=3.2.0", "BUILD_CONFIG_NAME=-arduino-esp8266",
            "mklittlefs",
        ], check=True)

    env.Replace(MKFSTOOL=str(tool))


configure_native_mklittlefs()
