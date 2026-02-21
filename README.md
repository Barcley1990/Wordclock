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
- `ESPAsyncTCP`
- `ESPAsyncWebServer`
- `ESPAsyncHTTPUpdateServer`
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

## First Boot / WiFi Provisioning

- Device tries stored WiFi credentials first.
- If connection fails, it starts AP mode:
- SSID: `wordclock-setup`
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
- `/settings.html`: Auto-brightness settings and software reset

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

### `POST /api/settings`

Form fields:
- `auto_brightness_enabled` (`1|0|true|false|on`)
- `auto_lux_low` (float, `>= 0`)
- `auto_lux_high` (float, `>= 0`)
- `auto_brightness_min` (`1..255`)
- `auto_brightness_max` (`1..255`)

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

