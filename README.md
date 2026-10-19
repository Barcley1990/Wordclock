# Wordclock_v2 (ESP8266 + PlatformIO)

Wordclock firmware for an ESP-WROOM-02 (ESP8266) based clock with:
- WS2812 LED word matrix
- DS1302 RTC
- BH1750 ambient light sensor
- WiFi captive portal onboarding
- Async web UI (time view, color settings, system settings)
- OTA firmware updates
- LittleFS-based persistent settings

## Features

- German 11x10 word layout (`Layout_De_11x10`)
- RTC-backed time display on LED matrix
- NTP sync on boot (when WiFi is available)
- WiFi auto-connect with fallback AP/captive portal
- mDNS support (`http://wordclock.local`)
- OTA update via `espota`
- WebSocket push for current time
- Web pages served from LittleFS (`data/`)
- Persistent appearance settings:
- Color
- Manual brightness
- Auto-brightness configuration
- Auto-brightness control:
- Sensor sampling every 100 ms
- Rolling median filter over 1000 ms (10 samples)
- Filtered ambient lux logging every 1000 ms
- Brightness mapping from ambient lux to LED brightness
- Selectable time-change animations: none (default), fadeout, Matrix, left-right

## Hardware

Target board: `esp_wroom_02` (ESP8266, 2 MB flash)

Pin mapping (from `include/mcal.h`):
- `MCAL_BOOT_PIN`: GPIO0
- `MCAL_SDA_PIN`: GPIO2
- `MCAL_WIFI_EN_PIN`: GPIO4
- `MCAL_LED_DIN_PIN`: GPIO5
- `MCAL_DAT_PIN`: GPIO12
- `MCAL_CLK_PIN`: GPIO13
- `MCAL_SCL_PIN`: GPIO14
- `MCAL_RST_PIN`: GPIO15
- `MCAL_LED_EN_PIN`: GPIO16
- `MCAL_ADC_PIN`: A0

## Software Stack

- Framework: Arduino (ESP8266)
- Build system: PlatformIO
- Filesystem: LittleFS
- Key libraries:
- `RTC` (Makuna)
- `NTPClient`
- `Adafruit NeoPixel`
- `BH1750`
- `ESP32Async/ESPAsyncTCP` 2.0.0
- `ESP32Async/ESPAsyncWebServer` 3.12.1
- `ESP8266HTTPUpdateServer` (from ESP8266 core, recovery only)
- `ArduinoOTA` (from ESP8266 core)

## Project Structure

- `src/main.cpp`: Application runtime, WiFi, web server, OTA, scheduling, settings persistence
- `src/wordclock.cpp`: LED matrix and RTC/sensor integration
- `src/logger.cpp`: Serial/Telnet logger
- `include/`: Headers, layouts, low-level mapping
- `data/`: Web assets for LittleFS (`index.html`, `color.html`, `settings.html`, JS/CSS)
- `platformio.ini`: Environment configuration

## Build and Flash

### 1. Build firmware

```bash
pio run -e esp_wroom_02
```

#### Apple Silicon Macs (without Rosetta)

Use the native ARM64 environment for firmware and filesystem builds:

```bash
pio run -e esp_wroom_02_macos_arm64
pio run -e esp_wroom_02_macos_arm64 -t buildfs --disable-auto-clean
```

If `pio` is not on your PATH, use `~/.platformio/penv/bin/pio` instead.
In the PlatformIO IDE, select `esp_wroom_02_macos_arm64` for the build/upload tasks.
The default `esp_wroom_02` environment remains available for other hosts.

The registry's macOS Xtensa compiler is an Intel executable, even though its
package lists ARM64 as supported. Without Rosetta it fails with `bad CPU type in
executable`. The ARM64 environment pins PlatformIO Espressif 8266 4.2.1
(Arduino core 3.1.2) and the native GCC 10.3 build from
[ESPHome's toolchain project](https://github.com/esphome-libs/xtensa-lx106-elf-toolchain/releases/tag/10.3.0-esphome.3).

The filesystem tool also needs a native build. `scripts/native_mklittlefs.py`
fetches mklittlefs 3.2.0 and its LittleFS submodule at verified, fixed commits,
then builds it with Apple's Command Line Tools. This runs only for filesystem
targets and caches the executable under `.pio/tools/`; the first build needs
Git, `make`, `clang` and internet access. It preserves LittleFS 2.5.1 and
`LFS_NAME_MAX=32`, as required by Arduino ESP8266 3.1.2. Prebuilt mklittlefs 4.x
images are incompatible with this core.

Outputs are `.pio/build/esp_wroom_02_macos_arm64/firmware.bin` and `littlefs.bin`.
Use the same ARM64 environment in the upload commands below when building on
Apple Silicon. `--disable-auto-clean` retains the firmware output when switching
to the filesystem target; it does not skip dependency checks.

Initial recovery deployment on 2026-10-18 (Apple Silicon, GCC 10.3):

- Firmware build: successful; 454,189 bytes flash sections, 36,536 bytes RAM (44.6%).
- Upload image: 458,336 bytes; MD5 `6a993503fc0817e393a2b3463f9a6713`.
- The device reports 2 MB physical/configured flash and 1,114,112 bytes of
  available OTA staging space. Another image of the same size occupies 458,752
  bytes in 4 KiB sectors, leaving 655,360 bytes (640 KiB) spare.
- Firmware uploaded successfully with ArduinoOTA from recovery; the device's
  reported MD5 matches the uploaded image.
- LittleFS image: 499,712 bytes, built and uploaded through recovery `/update`;
  extracted web assets were compared with `data/`, with appearance and logs
  retained in the deployment image.
- Animation, recovery-policy and settings UI regression tests passed.
- Device load test: all four animation choices ran for at least 75 seconds
  each, crossing real minute changes. 1,115 HTTP requests (including parallel
  Settings assets/API requests) and repeatedly reconnected WebSockets completed
  without a reboot or incomplete asset response. The application reported
  healthy afterward, with 33,544 bytes free heap. Browser navigation through
  Home, Settings and Color also worked during the test. The original Fadeout
  selection and appearance settings were retained.
- Recovery device test: a deliberate software reset within the first 60 seconds
  of application startup returned to latched recovery, where the embedded
  firmware/filesystem upload form remained available. This exercises the
  unfinished-startup policy; it does not inject a real watchdog fault.

The network dependencies are pinned as a compatible pair. WebServer 3.6.0
declares ESPAsyncTCP >=2.0.0, while this project previously forced 1.2.2.
After a reported hardware-watchdog reset during web browsing, that mismatch
was corrected and the unused async HTTP updater dependency removed. The
recovery status now includes boot exception details and heap diagnostics;
the firmware MD5 is cached at boot instead of scanning flash in an async
HTTP callback. The precise origin of the earlier watchdog reset has not
been conclusively reproduced.

Firmware, settings backup, upload log and device-test logs/scripts are saved
locally under `.pio/backups/wordclock-network-fix/` (ignored by Git). A passing
five-minute load test does not establish long-term stability; if recovery
recurs, save `/api/recovery/status` before resuming the application so the
original reset details are preserved.

Follow-up on 2026-10-19: the 3.2-second Matrix build was committed as
`b597337` and uploaded, but another web request triggered an allocator
exception (`umm_malloc_core`). A diagnostic build subsequently hit a hardware
watchdog while HTTP traffic and WebSocket connection changes were running.
Recovery stayed reachable after both failures. The earlier dependency-pair
correction alone therefore did not resolve the problem.

The current firmware pins `ESP32Async/ESPAsyncWebServer` 3.12.1, which includes
upstream fixes for HTTP request lifetime and WebSocket frame/connection handling
([HTTP fix](https://github.com/ESP32Async/ESPAsyncWebServer/pull/465),
[WebSocket fix](https://github.com/ESP32Async/ESPAsyncWebServer/pull/462)).
That upgrade alone also failed the split-frame test. The RTC crash record
identified `AsyncWebSocketClient::_onData` → `_queueControl` → `makeSharedBuffer`
→ the allocator. Source inspection found that a one-byte TCP fragment was
treated as a two-byte WebSocket header: the remaining byte count underflowed,
and mask/payload processing could read and overwrite adjacent memory.
Incomplete extended lengths and split control payloads were also unsafe.

`scripts/patch_async_websocket.py` applies the checked-in patch to exactly
3.12.1 before compilation. It verifies original/patched SHA-256 hashes and
refuses unexpected library sources. The build hook requires Git on `PATH`.
The fixed parser retains up to 14 header
bytes and 125 control-payload bytes per connection, waits for complete headers,
and bounds close-reason text by the frame length instead of `strlen()`.
The standalone header reader is in `patches/WebSocketFrameHeader.h`; host tests
exercise every header split, bytewise input, extended lengths and control-frame
validation. The normal web server also starts after hardware/settings initialization.

- Current firmware image: 454,896 bytes, MD5 `5c2173f92759bc3933c52b8ee38bbe1d`.
- RAM: 36,084 bytes (44.0%); flash sections: 450,737 bytes.
- Device OTA staging space: 1,114,112 bytes, leaving 640 KiB after sector-rounded
  staging of another image of the same size.
- Installed through the embedded recovery HTTP updater; firmware MD5 checked
  on-device before restarting the application. LittleFS was retained.
- Firmware/ELF and deployment/test logs: `.pio/backups/wordclock-ws-header-fix/`.
  The reproducer and decoded earlier crash are under
  `.pio/backups/wordclock-webserver-fix/`.
- Patched device test: passed over approximately seven minutes, with 360 seconds
  of HTTP load, 3,224 counted HTTP requests, 100 aborted downloads and 240
  WebSocket connections (including bytewise headers, extended lengths, split
  ping/close payloads and non-NUL-terminated close reasons). No reboot or corrupt
  web asset response; original appearance retained; final free heap 32,712 bytes.
  A separate 40-connection preflight also passed the former immediate reproducer.
- ArduinoOTA from normal mode with an open WebSocket also succeeded. The device
  rebooted into recovery with the expected MD5 and the embedded `/update` form
  reachable, then the clock application was resumed.
- The 3.2-second Matrix effect was selected temporarily for 75 seconds after
  that update. All 75 received clock messages were valid, spanning real minute
  changes without reboot; application health was confirmed. The original
  Left-right selection and identical persisted appearance file were restored.

### 2. Build and upload filesystem image

```bash
pio run -t uploadfs -e esp_wroom_02
```

### 3. Upload firmware

Serial:

```bash
pio run -t upload -e esp_wroom_02
```

OTA (configured in `platformio.ini`):

```bash
pio run -t upload -e esp_wroom_02
```

## Crash recovery and OTA

The updater starts **before** LittleFS, RTC, light sensor, logger and LED code.
Every boot provides a 10-second recovery/OTA window after WiFi initialization.
On an exception, hardware/software watchdog reset, or an unfinished application
startup, the device stays in recovery. A first software restart from older
firmware without a recovery record also stays in recovery.

- Open `/recovery` in normal mode and select **Stay in recovery / stop clock**.
  After the restart, `/update` accepts firmware and LittleFS images. The page is
  embedded in firmware and does not require a working filesystem.
- ArduinoOTA (`espota`, port 8266) remains available in recovery and normal mode.
- Every update attempt latches recovery. After a successful upload and reboot,
  inspect `/api/recovery/status` before selecting **Restart clock application**.
- The status endpoint reports mode, reset reason/EPC/exception cause, free heap,
  largest free block, application health, firmware MD5, sketch size, available
  OTA staging space, configured/physical flash size and WiFi address.
- Recovery version 3 also reports `reset_excvaddr` and `last_crash`. On a software
  exception/panic, a bounded RTC record saves the cause, address and candidate
  code addresses from the stack without allocating memory or writing flash.
  `last_crash.image` identifies the first eight MD5 digits of the crashing image;
  decode its addresses with that image's saved ELF, not a later build. The record
  persists through software restarts and occupies RTC user-memory bytes 288–499,
  separate from the recovery marker and eboot. A hardware-watchdog reset may
  bypass the crash callback, and power loss can erase RTC diagnostics.
- If stored WiFi fails, join **wordclock-recovery** and open **192.168.4.1**.
  The fallback AP runs alongside station mode and also supports both updaters.
- Holding the boot button for approximately five seconds now requests recovery;
  it no longer deliberately enters the UART downloader.

A checked record at RTC user-memory byte 256 (word offset 64) tracks startup and
recovery. It is outside eboot's first 128 bytes. After 60 seconds of application
operation the startup marker is cleared, but a later watchdog/exception still
triggers recovery. The RTC record is not durable across power loss; after a cold
start the boot window and crash reset detection still provide recovery.
`ATOMIC_FS_UPDATE` stages filesystem uploads before eboot installs them. The
application is suspended and LittleFS unmounted when an update starts.

This is application-level crash recovery, not an independent bootloader or
A/B rollback system. Failures in the recovery/network code itself, global
constructors before `setup()`, corrupt flash/bootloader, power loss during flash
installation or unavailable hardware/network can still require physical access.
Never remove this recovery path from a future image and do not upload an image
that does not fit both the current OTA staging space and the selected layout.

Recovery policy regression tests:

```bash
clang++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -Iinclude test/recovery_state_test.cpp -o /tmp/wordclock-recovery-test
/tmp/wordclock-recovery-test
```

## First Boot / WiFi Provisioning

- Device tries stored WiFi credentials first.
- If connection fails, it starts AP mode:
- SSID: `wordclock-recovery`
- Connect to AP and open `http://192.168.4.1`
- Enter WiFi credentials in captive portal.
- Device stores credentials and restarts.

## Web Interface

Base URL:
- `http://wordclock.local` (mDNS)
- or device IP

Pages:
- `/`: Home (current time via WebSocket)
- `/color.html`: Color and manual brightness
- `/settings.html`: Time-change animation, auto-brightness settings, error log and software reset

## Time-change animations

Select an effect under **Settings → Time Change Animation** and save it.
The selection persists in `/appearance.cfg` as `animation=<id>`. Existing
settings files without this key keep the default `none`.

| ID | Behavior |
| --- | --- |
| `none` | Immediate update, as before |
| `fadeout` | Fade the old time to black, then fade in the new time (1.2 s) |
| `matrix` | Staggered green columns fall downward with trails; the new time settles and returns to the selected color (3.2 s) |
| `left-right` | Occupied text rows alternate directions from top to bottom; empty rows do not count. The first occupied row moves left to right, the second right to left, the third left to right, etc. Old text slides out, then new text slides in from the opposite edge; directions are determined separately for each text (1.2 s) |

Transitions occur only when the displayed word text changes, normally every
five minutes. The four minute LEDs update immediately between text changes,
without starting an animation. Boot shows the current time immediately. The
minute LEDs fade during text transitions. Changing the effect during a transition
immediately shows the new time; the selected effect starts on the next text
change. Manual and automatic
brightness still apply to every frame.

`ClockAnimation::Animator` is a hardware-independent renderer for the current
11×10 layout and four minute LEDs. It uses fixed-size frames, no dynamic
allocation or delays, and renders at approximately 30 fps only during an effect
or when appearance changes. `loop()` services ArduinoOTA before rendering.
To add an effect, extend `Effect`, the `Options` registry and the renderer in
`include/clock_animation.h` / `src/clock_animation.cpp`. The dropdown gets its
options from the API, so it does not need a separate hard-coded effect list.

Both firmware and the LittleFS web assets need updating for this feature.
A full filesystem image replaces persisted settings and error logs as well as
web assets; back them up first if they need to be retained. Before any upload,
verify the firmware build, OTA space for the selected flash layout, and the
update route available on the running device. The independent recovery service
provides ArduinoOTA and the ESP8266 core HTTP updater at `/update`.

## HTTP API

### `GET /api/color`

Returns:
- `color` (`#RRGGBB`)
- `brightness` (manual brightness)
- `effective_brightness` (currently applied brightness)

### `POST /api/color`

Form fields:
- `color` (`#RRGGBB`, optional)
- `brightness` (`1..255`, optional)

Behavior:
- Applies updated appearance
- Persists to LittleFS

### `GET /api/settings`

Returns:
- `auto_brightness_enabled` (`true`/`false`)
- `auto_lux_low`
- `auto_lux_high`
- `auto_brightness_min`
- `auto_brightness_max`
- `ambient_filtered_lux`
- `animation`: `none`, `fadeout`, `matrix`, or `left-right`
- `animation_options`: available `{id, label}` pairs for the dropdown

### `POST /api/settings`

Form fields:
- `auto_brightness_enabled` (`1|0|true|false|on`)
- `auto_lux_low` (float, `>= 0`)
- `auto_lux_high` (float, `>= 0`)
- `auto_brightness_min` (`1..255`)
- `auto_brightness_max` (`1..255`)
- `animation` (optional effect ID; unknown IDs return HTTP 400)

Behavior:
- Validates and applies values
- Normalizes swapped min/max boundaries
- Persists to LittleFS

### `POST /api/reset`

Behavior:
- Returns success response
- Triggers software reset shortly after response

## Persistent Files (LittleFS)

- `/appearance.cfg`: appearance and auto-brightness configuration
- Web assets from `data/` are deployed to LittleFS

## Logging

- Serial logger at `115200`
- Optional Telnet logger (default port `23`)
- Ambient median lux is logged every 1000 ms

## Notes

- If web changes do not appear, re-run `uploadfs`.
- OTA upload requires reachable host/IP and matching OTA auth (if enabled).
- If mDNS does not resolve on your network, use the device IP.

## Animation regression tests (without hardware)

```bash
clang++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -Iinclude src/clock_animation.cpp test/animation_test.cpp \
  -o /tmp/wordclock-animation-test
/tmp/wordclock-animation-test
clang++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -Ipatches test/websocket_header_test.cpp -o /tmp/wordclock-websocket-header-test
/tmp/wordclock-websocket-header-test
node test/settings_test.mjs
```

These cover effect timing/direction, minute LEDs, default/boot behavior, frame
rate limiting, unchanged RTC polls, interrupted transitions, color changes,
`millis()` rollover, dropdown loading/saving and API error handling. They do not
replace a successful ESP8266 build or an OTA test on the device.

For a running device, the read-only hardware check exercises parallel page
loads, 100 deliberately aborted downloads and 240 WebSocket connections with
normal, split-frame and abrupt closes. It checks firmware MD5, monotonically
increasing uptime, full web asset contents and retained appearance settings:

```bash
python3 scripts/check_device_stability.py --host 192.168.178.62 \
  --md5 5c2173f92759bc3933c52b8ee38bbe1d --seconds 360 \
  --log /tmp/wordclock-stability.jsonl
```

Run this against the expected firmware in normal mode. It does not flash,
resume recovery or alter settings. On failure it tries to save the reset status
before another reboot can replace that evidence.
