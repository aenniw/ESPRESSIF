#pragma once

#define BLE_NAME "ESP-Ble"
#define BLE_PIN 223344

#define PIXELS_LEN  7
#ifdef ARDUINO_ARCH_ESP32
#define PIXELS_PIN  GPIO_NUM_21
#else
#define PIXELS_PIN  RX
#endif