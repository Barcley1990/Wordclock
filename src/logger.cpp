#include "logger.h"
#include <LittleFS.h>

/**
 * @brief Start the logger server.
 * @param port TCP port for Telnet logging.
 */
void Logger::begin(uint16_t port) {
  server = WiFiServer(port);
  server.begin();
  server.setNoDelay(true);
}

/**
 * @brief Handle incoming Telnet clients.
 */
void Logger::handle() {
  if (server.hasClient()) {
    if (!client || !client.connected()) {
      client = server.accept();
      Serial.println("[LOGGER] Telnet client connected");
      client.println("Telnet logger connected");
    } else {
      server.accept().stop(); // only 1 client
    }
  }
}

/**
 * @brief Set current log level.
 * @param level Minimum level to output.
 */
void Logger::setLevel(LogLevel level) {
  currentLevel = level;
}

/**
 * @brief Log error message.
 * @param msg Message to log.
 */
void Logger::error(const String& msg) { log(LOG_ERROR, "ERROR", msg); }
/**
 * @brief Log warning message.
 * @param msg Message to log.
 */
void Logger::warn (const String& msg) { log(LOG_WARN,  "WARN ", msg); }
/**
 * @brief Log info message.
 * @param msg Message to log.
 */
void Logger::info (const String& msg) { log(LOG_INFO,  "INFO ", msg); }
/**
 * @brief Log debug message.
 * @param msg Message to log.
 */
void Logger::debug(const String& msg) { log(LOG_DEBUG, "DEBUG", msg); }

/**
 * @brief Enable persistent error file logging.
 * @param path LittleFS path.
 */
void Logger::enableErrorFileLogging(const String& path) {
  errorFilePath = path;
}

/**
 * @brief Set timestamp provider for persisted log entries.
 * @param provider Function returning formatted timestamp.
 */
void Logger::setTimestampProvider(TimestampProvider provider) {
  timestampProvider = provider;
}

/**
 * @brief Internal log output.
 * @param level Message level.
 * @param prefix Prefix label.
 * @param msg Message content.
 */
void Logger::log(LogLevel level, const char* prefix, const String& msg) {
  if (level > currentLevel) return;

  String line = "[" + String(prefix) + "] " + msg;

  Serial.println(line);

  if (client && client.connected()) {
    client.println(line);
  }

  if (level == LOG_ERROR && errorFilePath.length() > 0) {
    String timestamp = String(millis()) + "ms";
    if (timestampProvider != nullptr) {
      const String provided = timestampProvider();
      if (provided.length() > 0) {
        timestamp = provided;
      }
    }

    File f = LittleFS.open(errorFilePath, "a");
    if (f) {
      f.println(String("[") + timestamp + "] " + line);
      f.close();
    }
  }
}
