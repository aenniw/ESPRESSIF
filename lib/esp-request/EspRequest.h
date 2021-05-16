#pragma once

#include <WString.h>
#include <functional>

class Request {
public:
    virtual String uri() const = 0;
    virtual String pathArg(unsigned int i) const = 0;

    void send(int code) { send(code, nullptr); };
    void send(int code, const char *content_type) { send(code, content_type, String("")); };
    virtual void send(int code, const char *content_type, const String &content)=0;
};

typedef std::function<void(Request *r)> EspRequestHandler;
