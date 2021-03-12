#include "commons.h"

#ifndef LOG_MAX_LEN
#define LOG_MAX_LEN 128
#endif

static Print *stream = nullptr;

void __log_init__(Print *printer) { stream = printer; }

void __log__(const __FlashStringHelper *fmt, ...) {
    if (!stream) return;
    char buf[LOG_MAX_LEN];
    sprintf_P(buf, PSTR("%010ld | "), millis());
    stream->print(buf);
    va_list args;
    va_start(args, fmt);
    vsnprintf_P(buf, LOG_MAX_LEN, (PGM_P) fmt, args);
    va_end(args);
    stream->println(buf);
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