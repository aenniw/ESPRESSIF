#include "commons.h"

#ifndef LOG_MAX_LEN
#define LOG_MAX_LEN 128
#endif

static Print *stream = nullptr;

void __log_init__(Print *printer) { stream = printer; }

void __log__(const char *format, ...) {
    if (!stream) return;
    static char loc_buf[64];
    char *temp = loc_buf;
    size_t len;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    len = vsnprintf(nullptr, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf)) {
        temp = (char *) malloc(len + 1);
        if (temp == nullptr) {
            return;
        }
    }
    vsnprintf(temp, len + 1, format, arg);
    ets_printf("%s", temp);
    va_end(arg);
    if (len >= sizeof(loc_buf)) {
        free(temp);
    }
}

long get_digit(const String &s) {
    if (!s || s.isEmpty()) {
        return -1;
    }
    for (char i : s) {
        if (!isDigit(i)) return -1;
    }
    return s.toInt();
}

float get_float(const String &s) {
    if (!s || s.isEmpty()) {
        return -1;
    }
    for (char i : s) {
        if (!isDigit(i) && i != '.') return -1;
    }
    return s.toFloat();
}

uint32_t get_chip_id() {
#ifdef ARDUINO_ARCH_ESP32
    uint32_t chipId = 0u;
    for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    return chipId;
#else
    return ESP.getChipId();
#endif
}

uint32_t get_random() {
#ifdef ARDUINO_ARCH_ESP32
    return esp_random();
#else
    return ESP.random();
#endif
}

std::string get_name(const std::string &prefix) {
    char hex[9] = "00000000";
    sprintf(hex, "%08x", get_random());
    return prefix + "-" + hex;
}
