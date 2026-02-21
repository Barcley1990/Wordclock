(() => {
  const form = document.getElementById("wifi-form");
  const statusEl = document.getElementById("status");

  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    const ssid = document.getElementById("ssid").value.trim();
    const pass = document.getElementById("pass").value;
    if (!ssid) {
      statusEl.textContent = "SSID is required.";
      return;
    }
    statusEl.textContent = "Saving credentials...";
    try {
      const body = new URLSearchParams({ ssid, pass });
      const res = await fetch("/wifi", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body,
      });
      if (res.ok) {
        statusEl.textContent = "Saved. Connecting...";
      } else {
        statusEl.textContent = "Failed to save credentials.";
      }
    } catch (err) {
      statusEl.textContent = "Network error.";
    }
  });
})();
