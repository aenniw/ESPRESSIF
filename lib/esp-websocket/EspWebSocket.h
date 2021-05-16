#pragma once

#include <commons.h>
#include <EspRequest.h>
#include <WebSocketsServer.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif

#ifndef EVENT_MSG_LEN
#define EVENT_MSG_LEN 256
#endif

class EventRequest : public Request {
private:
    WebSocketsServer &ws;
    std::vector<String> &topicArgs;
    const String &_topic;
    const uint8_t client;
    const bool broadcast;
    const HTTPMethod method;
public:
    explicit EventRequest(WebSocketsServer &ws, const uint8_t num, const String &topic, HTTPMethod method,
                          const bool broadcast, std::vector<String> &topicArgs) :
            ws(ws), topicArgs(topicArgs), _topic(topic), client(num), broadcast(broadcast), method(method) {}

    String uri() const override;
    String pathArg(unsigned int i) const override;
    void send(int code, const char *content_type, const String &content) override;
};

class EventHandler {
private:
    EspRequestHandler fn;
    Uri *topic;
    HTTPMethod method;
    bool broadcast;
protected:
    EventHandler *_next = nullptr;
    static std::vector<String> topicArgs;
public:

    EventHandler(EspRequestHandler fn, const Uri &topic, HTTPMethod method, bool broadcast) :
            fn(std::move(fn)), topic(topic.clone()), method(method), broadcast(broadcast) {}

    bool canHandle(HTTPMethod m, const String &to);
    void handle(WebSocketsServer &ws, uint8_t num, const String &to);

    EventHandler *next();
    void next(EventHandler *e);

    virtual ~EventHandler() = default;
};

class EspWebSocket : public Service {
private:
    WebSocketsServer ws;
    EventHandler *_firstHandler = nullptr, *_lastHandler = nullptr;
protected:
    void onEvent(uint8_t c, WStype_t type, uint8_t *payload, size_t length);

    void addRequestHandler(EventHandler *handler);

public:
    explicit EspWebSocket(uint16_t p, const char *secret = nullptr);;

    void begin() override;;

    EspWebSocket &
    on(HTTPMethod m, const Uri &uri, EspRequestHandler fn, bool broadcast = true);

    EspWebSocket &serve(Subscriber<EspWebSocket> &s);

    void cycle() override;;
};