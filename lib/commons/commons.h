#pragma once

#include <Arduino.h>

#ifdef DEBUG_ESP_PORT
#define log_init(s, b) (*(s)).begin(b);
#if defined(ARDUINO_ARCH_ESP8266)
    void __log_init__(Print *stream);
    void __log__(const char *fmt, ...);

    #define log_init(s, b) (*(s)).begin(b);__log_init__(s)
    #define log_format(letter, format)  "[" #letter "][%s:%u] %s(): " format "\r\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__
    #define log_i(fmt, ...) __log__(log_format(I, fmt), ##__VA_ARGS__)
    #define log_d(fmt, ...) __log__(log_format(D, fmt), ##__VA_ARGS__)
    #define log_w(fmt, ...) __log__(log_format(W, fmt), ##__VA_ARGS__)
    #define log_e(fmt, ...) __log__(log_format(E, fmt), ##__VA_ARGS__)
    #define log_v(fmt, ...) __log__(log_format(V, fmt), ##__VA_ARGS__)
#endif
#else
#define log_init(...)
#if defined(ARDUINO_ARCH_ESP8266)
    #define log_i(...)
    #define log_d(...)
    #define log_w(...)
    #define log_e(...)
    #define log_v(...)
#endif
#endif

template<class T>
class Subscriber {
public:
    virtual void subscribe(T &o) = 0;
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

uint32_t get_chip_id();
uint32_t get_random();
std::string get_name(const std::string &prefix);