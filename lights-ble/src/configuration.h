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

#ifndef PIXELS_STRANDS
#define PIXELS_STRANDS 3
#endif

#ifndef FW_VERSION
#define FW_VERSION "unknown"
#endif

#ifndef HW_VERSION
#define HW_VERSION "unknown"
#endif

#if PIXELS_STRANDS > 3
#error Exceeded max number of individual pixel strands
#endif