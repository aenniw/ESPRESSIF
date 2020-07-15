#include <uri/UriRegex.h>
#include <detail/mimetable.h>
#include "EspServer.h"

String mineType(const String &p) {
#if defined(ARDUINO_ARCH_ESP8266)
    return mime::getContentType(p);
#elif defined(ARDUINO_ARCH_ESP32)
    using mime::mimeTable;
    char buff[sizeof(mimeTable[0].mimeType)];
    // Check all entries but last one for match, return if found
    for (size_t i=0; i < sizeof(mimeTable)/sizeof(mimeTable[0])-1; i++) {
        strcpy_P(buff, mimeTable[i].endsWith);
        if (p.endsWith(buff)) {
            strcpy_P(buff, mimeTable[i].mimeType);
            return String(buff);
        }
    }
    // Fall-through and just return default type
    strcpy_P(buff, mimeTable[sizeof(mimeTable)/sizeof(mimeTable[0])-1].mimeType);
    return String(buff);
#endif
}

const char *mineType(uint8_t t) {
    return mime::mimeTable[t].mimeType;
}

void EspServer::not_found() {
    LOG("N/A | %d | %s", server.method(), server.uri().c_str());

    if (cors && server.method() == HTTP_OPTIONS) {
        server.sendHeader(F("Access-Control-Max-Age"), F("10000"));
        server.sendHeader(F("Access-Control-Allow-Methods"), F("DELETE,PUT,POST,GET,OPTIONS"));
        server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
        server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
        server.send(204);
    } else if (secret) {
        server.send(401);
    } else {
        server.send(404);
    }
}

void EspServer::begin() {
    server.onNotFound(std::bind(&EspServer::not_found, this));
    server.begin();
}

EspServer &
EspServer::on(const HTTPMethod method, const __FlashStringHelper *uri, const RestHandler &onRequest,
              const RestHandler &onUpload, const bool checkAuth) {
    SERVER::THandlerFunction upload = [=]() { onUpload(&request); };
    server.on(UriRegex(uri), method, [=]() {
        LOG("%d | %s", method, server.uri().c_str());
        bool auth = !checkAuth || !(user && secret);
        if (!auth) {
#ifdef ARDUINO_ARCH_ESP8266
            if (digest)
                auth |= server.authenticateDigest(user, secret);
            else
#endif
                auth |= server.authenticate(user, secret);
        }
        if (!auth)
            server.requestAuthentication(digest ? DIGEST_AUTH : BASIC_AUTH, realm);
        else
            onRequest(&request);
    }, !onUpload ? nullptr : upload);
    return *this;
}

EspServer &EspServer::serveStatic(const char *uri, fs::FS &fs, const char *path, const char *cache_header) {
    server.serveStatic(uri, fs, path, cache_header);
    return *this;
}

EspServer &EspServer::serve(Subscriber<EspServer> &s) {
    s.subscribe(*this);
    return *this;
}

void EspServer::cycle() {
    server.handleClient();
}

String Request::uri() const {
    return proxy.uri();
}

String Request::pathArg(unsigned int i) const {
    return proxy.pathArg(i);
}

void Request::send(int code, const char *content_type, const String &content) {
    proxy.send(code, content_type, content);
}

bool Request::hasArg(const String &name) const { return proxy.hasArg(name); }

String Request::arg(const String &name) const { return proxy.arg(name); }
