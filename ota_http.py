import requests

ESP_IP = "wordclock.local"
BIN_FILE = ".pio/build/esp_wroom_02/firmware.bin"

url = f"http://{ESP_IP}/update"

with open(BIN_FILE, "rb") as f:
    files = {"update": f}
    r = requests.post(url, files=files, timeout=10)

r.raise_for_status()
print("OTA erfolgreich:", r.text)