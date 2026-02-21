#pragma once
#include <ESP8266WiFi.h>

enum LogLevel {
  LOG_ERROR = 0,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG
};

class Logger {
public:
  /**
   * @brief Start the logger server.
   * @param port TCP port for Telnet logging.
   */
  void begin(uint16_t port = 23);
  /**
   * @brief Handle incoming Telnet clients.
   */
  void handle();

  /**
   * @brief Set current log level.
   * @param level Minimum level to output.
   */
  void setLevel(LogLevel level);

  /**
   * @brief Log error message.
   * @param msg Message to log.
   */
  void error(const String& msg);
  /**
   * @brief Log warning message.
   * @param msg Message to log.
   */
  void warn (const String& msg);
  /**
   * @brief Log info message.
   * @param msg Message to log.
   */
  void info (const String& msg);
  /**
   * @brief Log debug message.
   * @param msg Message to log.
   */
  void debug(const String& msg);

private:
  WiFiServer server{23};
  WiFiClient client;
  LogLevel currentLevel = LOG_INFO;

  /**
   * @brief Internal log output.
   * @param level Message level.
   * @param prefix Prefix label.
   * @param msg Message content.
   */
  void log(LogLevel level, const char* prefix, const String& msg);
};
