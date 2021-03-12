#pragma once

#define AUTH_USER "root"
#define AUTH_PASS "toor"

#define WIFI_SSID "ESP-AP"
#define WIFI_PSK "PA-PSW"
#define WIFI_MODE WIFI_AP

#define PIXELS_LEN  7
#ifdef ARDUINO_ARCH_ESP32
#define PIXELS_PIN  GPIO_NUM_21
#else
#define PIXELS_PIN  RX
#endif