(() => {
  const form = document.getElementById("color-form");
  const statusEl = document.getElementById("status");
  const colorInput = document.getElementById("color");
  const brightnessInput = document.getElementById("brightness");
  const brightnessValueEl = document.getElementById("brightness-value");

  const syncBrightnessText = () => {
    brightnessValueEl.textContent = brightnessInput.value;
  };

  brightnessInput.addEventListener("input", syncBrightnessText);

  const loadSettings = async () => {
    statusEl.textContent = "Loading current settings...";
    try {
      const res = await fetch("/api/color", { cache: "no-store" });
      if (!res.ok) {
        statusEl.textContent = "Failed to load settings.";
        return;
      }
      const data = await res.json();
      if (data && typeof data.color === "string") {
        colorInput.value = data.color;
      }
      if (data && Number.isInteger(data.brightness)) {
        brightnessInput.value = String(data.brightness);
      }
      syncBrightnessText();
      statusEl.textContent = "Current settings loaded.";
    } catch (err) {
      statusEl.textContent = "Network error while loading settings.";
    }
  };

  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    statusEl.textContent = "Saving settings...";
    try {
      const body = new URLSearchParams({
        color: colorInput.value,
        brightness: brightnessInput.value,
      });
      const res = await fetch("/api/color", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body,
      });
      if (!res.ok) {
        statusEl.textContent = "Failed to save settings.";
        return;
      }
      statusEl.textContent = "Settings saved.";
    } catch (err) {
      statusEl.textContent = "Network error while saving settings.";
    }
  });

  loadSettings();
})();
