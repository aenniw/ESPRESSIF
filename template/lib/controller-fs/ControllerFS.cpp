#include "ControllerFS.h"
#include <ArduinoJson.h>
#ifdef ARDUINO_ARCH_ESP32
#include <detail/mimetable.h>
#endif

String ControllerFS::getFileName(Request *request) {
    return request->uri().substring(URI_OFFSET);
}

void ControllerFS::ls(Request *request) const {
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
    request->send(200, mineType(mime::json), response);
}

void ControllerFS::read(Request *request) const {
    if (!fs.read(getFileName(request), [&](File &f) {
        request->streamFile(f);
    })) {
        request->send(404);
    }
}

void ControllerFS::mkdir(Request *request) const {
    if (fs.mkdir(getFileName(request)))
        return request->send(200);
    request->send(409);
}

void ControllerFS::writeUrlEncoded(Request *request) const {
    if (request->hasArg(F("plain"))) {
        if (fs.write(getFileName(request), [&](File &f) {
            f.print(request->arg(F("plain")));
        }))
            return request->send(200);
    }
    request->send(500);
}

void ControllerFS::rm(Request *request) const {
    if (fs.rm(getFileName(request)))
        request->send(200);
    else
        request->send(404);
}

void ControllerFS::subscribe(EspServer &rest) const {
    rest.on(HTTP_GET, F(R"(^\/api\/v1\/fs\/(.*\/)*$)"),
            std::bind(&ControllerFS::ls, this, std::placeholders::_1));
    rest.on(HTTP_GET, F(R"(^\/api\/v1\/fs\/.*[^\/]$)"),
            std::bind(&ControllerFS::read, this, std::placeholders::_1));
    rest.on(HTTP_POST, F(R"(^\/api\/v1\/fs\/.+\/$)"),
            std::bind(&ControllerFS::mkdir, this, std::placeholders::_1));
    rest.on(HTTP_POST, F(R"(^\/api\/v1\/fs\/.*[^\/]$)"),
            std::bind(&ControllerFS::writeUrlEncoded, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, F(R"(^\/api\/v1\/fs\/.*$)"),
            std::bind(&ControllerFS::rm, this, std::placeholders::_1));
}
