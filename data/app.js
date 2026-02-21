(() => {
  const statusEl = document.getElementById("status");
  const timeEl = document.getElementById("time");
  let socket;

  const connect = () => {
    const proto = location.protocol === "https:" ? "wss" : "ws";
    socket = new WebSocket(`${proto}://${location.host}/ws`);

    socket.onopen = () => {
      statusEl.textContent = "Connected";
    };

    socket.onclose = () => {
      statusEl.textContent = "Disconnected — retrying…";
      setTimeout(connect, 1500);
    };

    socket.onerror = () => {
      statusEl.textContent = "Error";
    };

    socket.onmessage = (evt) => {
      timeEl.textContent = evt.data;
    };
  };

  connect();
})();
