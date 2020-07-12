#pragma once

#include <Service.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#define SERVER ESP8266WebServer
#elif defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#define SERVER WebServer
#endif

class Request {
private:
    SERVER &proxy;
public:
    explicit Request(SERVER &s) : proxy(s) {}

    String pathArg(unsigned int i) {
        return proxy.pathArg(i);
    };

    void send(int code, const char *content_type = nullptr, const String &content = String("")) {
        proxy.send(code, content_type, content);
    };
};

typedef std::function<void(Request *)> RestHandler;

const char *mineType(uint8_t t);

class EspServer : public Service {
private:
    SERVER server;
    Request request;

    const char *realm = "esp-realm";
    const char *user = nullptr;
    const char *secret = nullptr;

    bool digest = false, cors = false;

private:
    void not_found();

public:
    explicit EspServer(const uint16_t port = 80, const char *user = nullptr, const char *secret = nullptr,
                       bool digest = false, bool cors = false)
            : server(port), request(server), user(user), secret(secret), digest(digest), cors(cors) {}

    void begin() override;;

    EspServer &
    on(HTTPMethod method, const __FlashStringHelper *uri, const RestHandler &onRequest, bool checkAuth = true);

    EspServer &serveStatic(const char *uri, fs::FS &fs, const char *path, const char *cache_header = nullptr);

    EspServer &serve(Subscriber<EspServer> &s);

    void cycle() override;
};

