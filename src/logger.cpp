#include "logger.h"

void Logger::begin(uint16_t port) {
  server = WiFiServer(port);
  server.begin();
  server.setNoDelay(true);
}

void Logger::handle() {
  if (server.hasClient()) {
    if (!client || !client.connected()) {
      client = server.accept();
      Serial.println("[LOGGER] Telnet client verbunden");
      client.println("Telnet Logger verbunden");
    } else {
      server.accept().stop(); // nur 1 Client
    }
  }
}

void Logger::setLevel(LogLevel level) {
  currentLevel = level;
}

void Logger::error(const String& msg) { log(LOG_ERROR, "ERROR", msg); }
void Logger::warn (const String& msg) { log(LOG_WARN,  "WARN ", msg); }
void Logger::info (const String& msg) { log(LOG_INFO,  "INFO ", msg); }
void Logger::debug(const String& msg) { log(LOG_DEBUG, "DEBUG", msg); }

void Logger::log(LogLevel level, const char* prefix, const String& msg) {
  if (level > currentLevel) return;

  String line = "[" + String(prefix) + "] " + msg;

  Serial.println(line);

  if (client && client.connected()) {
    client.println(line);
  }
}