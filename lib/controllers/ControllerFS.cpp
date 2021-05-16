#include "ControllerFS.h"
#include <ArduinoJson.h>
#ifdef ARDUINO_ARCH_ESP32
#include <detail/mimetable.h>
#include <uri/UriRegex.h>
#define DIR_OFFSET 0
#else
#include <uri/UriGlob.h>
#define DIR_OFFSET 1
#endif

String ControllerFS::getFileName(RestRequest *request, unsigned int offset) {
    auto uri = request->uri();
    return uri.substring(URI_OFFSET, uri.length() - offset);
}

void ControllerFS::ls(RestRequest *request) const {
    StaticJsonDocument<512> doc;

    JsonArray files = doc.createNestedArray(F("file"));
    JsonArray dirs = doc.createNestedArray(F("dir"));
    fs.ls(getFileName(request), [&](const String &n, const bool dir) {
        if (dir)
            dirs.add(n);
        else
            files.add(n);
    });

    if (!files.size() && !dirs.size()) {
        return request->send(404);
    }

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerFS::read(RestRequest *request) const {
    if (!fs.read(getFileName(request), [&](File &f) {
        request->streamFile(f);
    })) {
        request->send(404);
    }
}

void ControllerFS::mkdir(RestRequest *request) const {
    if (fs.mkdir(getFileName(request, DIR_OFFSET)))
        return request->send(200);
    request->send(409);
}

void ControllerFS::writeUrlEncoded(RestRequest *request) const {
    if (request->hasArg(F("plain"))) {
        if (fs.write(getFileName(request), [&](File &f) {
            f.print(request->arg(F("plain")));
        }))
            return request->send(200);
    }
    request->send(500);
}

void ControllerFS::rm(RestRequest *request) const {
    if (fs.rm(getFileName(request)))
        request->send(200);
    else
        request->send(404);
}

void ControllerFS::subscribe(EspServer &rest) {
#ifdef ARDUINO_ARCH_ESP32
    rest.on(HTTP_GET, UriRegex(F(R"(^\/api\/v1\/fs\/(.*\/)*$)")),
            std::bind(&ControllerFS::ls, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriRegex(F(R"(^\/api\/v1\/fs\/.*[^\/]$)")),
            std::bind(&ControllerFS::read, this, std::placeholders::_1));
    rest.on(HTTP_POST, UriRegex(F(R"(^\/api\/v1\/fs\/.+\/$)")),
            std::bind(&ControllerFS::mkdir, this, std::placeholders::_1));
    rest.on(HTTP_POST, UriRegex(F(R"(^\/api\/v1\/fs\/.*[^\/]$)")),
            std::bind(&ControllerFS::writeUrlEncoded, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, UriRegex(F(R"(^\/api\/v1\/fs\/.*$)")),
            std::bind(&ControllerFS::rm, this, std::placeholders::_1));
#else
    rest.on(HTTP_GET, Uri(F("/api/v1/fs/")),
            std::bind(&ControllerFS::ls, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriGlob(F("/api/v1/fs/*/")),
            std::bind(&ControllerFS::ls, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriGlob(F("/api/v1/fs/*")),
            std::bind(&ControllerFS::read, this, std::placeholders::_1));
    rest.on(HTTP_POST, UriGlob(F("/api/v1/fs/*/")),
            std::bind(&ControllerFS::mkdir, this, std::placeholders::_1));
    rest.on(HTTP_POST, UriGlob(F("/api/v1/fs/*")),
            std::bind(&ControllerFS::writeUrlEncoded, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, UriGlob(F("/api/v1/fs/*")),
            std::bind(&ControllerFS::rm, this, std::placeholders::_1));
#endif
}
