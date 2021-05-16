#pragma once

#define BLE_MANUFACTURER "0242ac130003"

#ifndef BLE_NAME
#define BLE_NAME get_name("Ambient")
#endif

#ifndef BLE_SECRET
#define BLE_SECRET get_chip_id()
#endif

#define PIXELS_CURRENT      1000
#define PIXELS_POTENTIAL    5

#ifndef PIXELS_LEN
#define PIXELS_LEN          40
#endif

#ifdef ARDUINO_ARCH_ESP32
#define PIXELS_PIN  GPIO_NUM_21
#else
#define PIXELS_PIN  RX
#endif

#ifndef FW_VERSION
#define FW_VERSION "unknown"
#endif

#ifndef HW_VERSION
#define HW_VERSION "unknown"
#endif