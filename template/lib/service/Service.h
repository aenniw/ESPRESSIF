#pragma once

#include <Arduino.h>

#ifdef __DEBUG__
void __log__(const __FlashStringHelper *fmt, ...);
void __log_init__(Print *stream);
#define LOG_INIT(f, s) f;__log_init__(s)
#define LOG(fmt, ...) __log__(F(fmt), ##__VA_ARGS__)
#else
#define LOG_INIT(...)
#define LOG(...)
#endif

template<class T>
class Subscriber {
public:
    virtual void subscribe(T &rest) const = 0;
};

class Service {
public:
    virtual void begin() {};

    virtual void cycle() = 0;
};