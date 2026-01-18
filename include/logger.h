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
  void begin(uint16_t port = 23);
  void handle();

  void setLevel(LogLevel level);

  void error(const String& msg);
  void warn (const String& msg);
  void info (const String& msg);
  void debug(const String& msg);

private:
  WiFiServer server{23};
  WiFiClient client;
  LogLevel currentLevel = LOG_INFO;

  void log(LogLevel level, const char* prefix, const String& msg);
};