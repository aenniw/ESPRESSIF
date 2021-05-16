#pragma once

#include <mime.h>
#include <commons.h>
#include <EspRequest.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#define SERVER ESP8266WebServer
#elif defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#define SERVER WebServer
#endif

class RestRequest : public Request {
private:
    SERVER &proxy;
public:
    explicit RestRequest(SERVER &s) : proxy(s) {}

    using Request::send;

    String uri() const override;
    String pathArg(unsigned int i) const override;
    void send(int code, const char *content_type, const String &content) override;

    bool hasArg(const String &name) const;
    String arg(const String &name) const;

    template<typename T>
    size_t streamFile(T &file) const {
#if defined(ARDUINO_ARCH_ESP8266)
        return proxy.streamFile(file, mimeType(file.fullName()), HTTP_GET);
#elif defined(ARDUINO_ARCH_ESP32)
        return proxy.streamFile(file, mimeType(file.name()));
#endif
    }

};

typedef std::function<void(RestRequest *)> RestHandler;

class EspServer : public Service {
private:
    SERVER server;
    RestRequest request;

    const char *realm = "esp-realm";
    const char *user = nullptr;
    const char *secret = nullptr;

    bool digest = false, cors = false;
    Predicate<String> *bearerValidator = nullptr;
private:
    void not_found();
    bool validate_bearer();

public:
    explicit EspServer(const uint16_t port = 80, const char *user = nullptr, const char *secret = nullptr,
                       bool digest = false, bool cors = false)
            : server(port), request(server), user(user), secret(secret), digest(digest), cors(cors) {}

    void begin() override;;

    EspServer &
    on(HTTPMethod method, const Uri &uri, const RestHandler &onRequest, const RestHandler &onUpload = nullptr,
       bool checkAuth = true);

    EspServer &serveStatic(const char *uri, fs::FS &fs, const char *path, const char *cache_header = nullptr);

    EspServer &serve(Subscriber<EspServer> &s);

    EspServer &setBearerValidator(Predicate<String> *validator);

    void cycle() override;
};

