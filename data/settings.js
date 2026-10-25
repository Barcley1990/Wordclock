(() => {
  const animationForm = document.getElementById("animation-form");
  const animationEl = document.getElementById("animation");
  const animationSaveEl = document.getElementById("animation-save");
  const animationStatusEl = document.getElementById("animation-status");
  const animationTestEl = document.getElementById("animation-test");
  const autoForm = document.getElementById("auto-form");
  const autoStatusEl = document.getElementById("auto-status");
  const autoEnabledEl = document.getElementById("auto-enabled");
  const autoLuxLowEl = document.getElementById("auto-lux-low");
  const autoLuxHighEl = document.getElementById("auto-lux-high");
  const autoBrightnessMinEl = document.getElementById("auto-brightness-min");
  const autoBrightnessMaxEl = document.getElementById("auto-brightness-max");
  const errorLogForm = document.getElementById("errorlog-form");
  const errorLogStatusEl = document.getElementById("errorlog-status");

  const form = document.getElementById("reset-form");
  const statusEl = document.getElementById("reset-status");

  const loadSettings = async () => {
    autoStatusEl.textContent = "Loading settings...";
    try {
      const res = await fetch("/api/settings", { cache: "no-store" });
      if (!res.ok) {
        autoStatusEl.textContent = "Failed to load settings.";
        animationStatusEl.textContent = "Failed to load animation. Reload to retry.";
        return;
      }

      const data = await res.json();
      if (Array.isArray(data.animation_options) && data.animation_options.length > 0) {
        animationEl.replaceChildren();
        data.animation_options.forEach(({ id, label }) => {
          const option = document.createElement("option");
          option.value = id;
          option.textContent = label;
          animationEl.appendChild(option);
        });
        animationEl.value = data.animation || "none";
        animationEl.disabled = false;
        animationSaveEl.disabled = false;
        animationTestEl.disabled = false;
        animationStatusEl.textContent = "Animation loaded.";
      } else {
        animationStatusEl.textContent = "Animation requires updated firmware.";
      }
      autoEnabledEl.checked = Boolean(data.auto_brightness_enabled);
      if (typeof data.auto_lux_low === "number") {
        autoLuxLowEl.value = String(data.auto_lux_low);
      }
      if (typeof data.auto_lux_high === "number") {
        autoLuxHighEl.value = String(data.auto_lux_high);
      }
      if (Number.isInteger(data.auto_brightness_min)) {
        autoBrightnessMinEl.value = String(data.auto_brightness_min);
      }
      if (Number.isInteger(data.auto_brightness_max)) {
        autoBrightnessMaxEl.value = String(data.auto_brightness_max);
      }
      autoStatusEl.textContent = "Settings loaded.";
    } catch (err) {
      autoStatusEl.textContent = "Network error.";
      animationStatusEl.textContent = "Failed to load animation. Reload to retry.";
    }
  };

  animationEl.addEventListener("change", async () => {
    if (!animationTestEl.checked || animationEl.disabled) return;
    // Serialize requests so rapid selections cannot arrive out of order.
    animationEl.disabled = true;
    animationStatusEl.textContent = "Starting test...";
    try {
      const res = await fetch("/api/animation/preview", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: new URLSearchParams({ animation: animationEl.value }),
      });
      animationStatusEl.textContent = res.ok
        ? "Test started on the clock. Selection has not been saved."
        : "Failed to start test. Please retry.";
    } catch (err) {
      animationStatusEl.textContent = "Network error. Please retry.";
    } finally {
      animationEl.disabled = false;
    }
  });

  animationForm.addEventListener("submit", async (e) => {
    e.preventDefault();
    animationSaveEl.disabled = true;
    animationStatusEl.textContent = "Saving animation...";
    try {
      const res = await fetch("/api/settings", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: new URLSearchParams({ animation: animationEl.value }),
      });
      animationStatusEl.textContent = res.ok
        ? "Saved. Plays at the next text change."
        : "Failed to save animation. Please retry.";
    } catch (err) {
      animationStatusEl.textContent = "Network error. Please retry.";
    } finally {
      animationSaveEl.disabled = false;
    }
  });

  autoForm.addEventListener("submit", async (e) => {
    e.preventDefault();
    autoStatusEl.textContent = "Saving settings...";

    try {
      const body = new URLSearchParams({
        auto_brightness_enabled: autoEnabledEl.checked ? "1" : "0",
        auto_lux_low: autoLuxLowEl.value,
        auto_lux_high: autoLuxHighEl.value,
        auto_brightness_min: autoBrightnessMinEl.value,
        auto_brightness_max: autoBrightnessMaxEl.value,
      });

      const res = await fetch("/api/settings", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body,
      });

      if (!res.ok) {
        autoStatusEl.textContent = "Failed to save settings.";
        return;
      }

      autoStatusEl.textContent = "Settings saved.";
    } catch (err) {
      autoStatusEl.textContent = "Network error.";
    }
  });

  errorLogForm.addEventListener("submit", (e) => {
    e.preventDefault();
    errorLogStatusEl.textContent = "Starting download...";
    window.location.href = "/api/logs/errors";
    setTimeout(() => {
      errorLogStatusEl.textContent = "If available, download started.";
    }, 400);
  });

  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    statusEl.textContent = "Resetting controller...";

    try {
      const res = await fetch("/api/reset", {
        method: "POST",
      });

      if (!res.ok) {
        statusEl.textContent = "Reset request failed.";
        return;
      }

      statusEl.textContent = "Reset triggered.";
    } catch (err) {
      statusEl.textContent = "Network error.";
    }
  });

  loadSettings();
})();
