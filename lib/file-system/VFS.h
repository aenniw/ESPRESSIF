#pragma once

#if defined(ARDUINO_ARCH_ESP8266)
#include <LittleFS.h>
#define VFS LittleFS
#elif defined(ARDUINO_ARCH_ESP32)
#include <SPIFFS.h>
#define VFS SPIFFS
#endif