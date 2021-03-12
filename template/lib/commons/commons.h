#pragma once

#include <Arduino.h>

#ifdef DEBUG_ESP_PORT
void __log__(const __FlashStringHelper *fmt, ...);
void __log_init__(Print *stream);
#define LOG_INIT(s, b) (*s).begin(b);__log_init__(s)
#define LOG(fmt, ...) __log__(F(fmt), ##__VA_ARGS__)
#else
#define LOG_INIT(...)
#define LOG(...)
#endif

template<class T>
class Subscriber {
public:
    virtual void subscribe(T &rest) = 0;
};

template<typename T>
class Predicate {
public:
    virtual bool test(T &o) const = 0;
};

class Service {
public:
    virtual void begin() {};

    virtual void cycle() = 0;
};

long get_digit(const String &s);
float get_float(const String &s);