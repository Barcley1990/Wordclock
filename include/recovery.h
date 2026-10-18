#pragma once
#include <Arduino.h>

// Independent of Wordclock, sensor drivers and LittleFS web assets.
namespace Recovery {
void begin(void (*suspendApplication)());
void handle();
bool applicationAllowed();
void applicationStarting();
void applicationHealthy();
bool enter();
String statusJson();
const char* page();
} // namespace Recovery
