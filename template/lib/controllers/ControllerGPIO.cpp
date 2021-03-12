#include "ControllerGPIO.h"
#include <ArduinoJson.h>
#include <EspServer.h>
#include <uri/UriBraces.h>
#ifdef ARDUINO_ARCH_ESP32
#include <analogWrite.h>
#include <detail/mimetable.h>
#endif

using namespace gpio;

gpio::mode ControllerGPIO::get_mode(const String &s) const {
    if (s.equalsIgnoreCase(F("INPUT")))
        return IN;
    if (s.equalsIgnoreCase(F("OUTPUT")))
        return OUT;
    return NONE;
}

gpio::type ControllerGPIO::get_type(const String &s) const {
    if (s.equalsIgnoreCase(F("ANALOG")))
        return LINEAR;
    if (s.equalsIgnoreCase(F("DIGITAL")))
        return DIGITAL;
    return INVALID;
}

void ControllerGPIO::begin() {
    fs.ls(base_path, [this](const String &n, const bool dir) {
        if (!dir) return;

        auto pin = (uint8_t) n.toInt();
        fs.read(base_path + n + F("/type"), [this, &pin, &n](File &t) {
            const auto gpio_type = (gpio::type) t.parseInt();
            fs.read(base_path + n + F("/value"), [&](File &v) {
                if (gpio_type == LINEAR) {
                    analogWrite(pin, v.parseInt());
                } else if (gpio_type == DIGITAL) {
                    digitalWrite(pin, (uint8_t) v.parseInt());
                }
            });
        });

        fs.read(base_path + n + F("/mode"), [&](File &m) {
            const auto mode = (uint8_t) m.parseInt();
            pinMode(pin, mode);
            LOG("pinMode %d %d", pin, mode);
        });
    });
}

void ControllerGPIO::rm(Request *request) const {
    request->send(fs.rm(base_path + request->pathArg(0)) ? 200 : 404);
}

void ControllerGPIO::mode(Request *request) const {
    const auto pin = get_digit(request->pathArg(0));
    const auto mode = get_mode(request->pathArg(1));

    if (pin < 0 || mode == NONE) {
        return request->send(400);
    }

    pinMode(pin, mode);
    fs.write(base_path + pin + F("/mode"), [&](File &f) { f.print(mode); });
    request->send(200);
}

void ControllerGPIO::toggle(Request *request) const {
    const auto pin = get_digit(request->pathArg(0));
    const auto value = (uint8_t) !digitalRead(pin);

    if (pin < 0) {
        return request->send(400);
    }

    digitalWrite(pin, value);
    fs.write(base_path + pin + F("/value"), [&](File &f) { f.print(value); });
    fs.write(base_path + pin + F("/type"), [&](File &f) { f.print(DIGITAL); });
    request->send(200);
}

void ControllerGPIO::write(Request *request) const {
    const auto pin = get_digit(request->pathArg(0));
    const auto type = get_type(request->pathArg(1));
    const auto value = get_digit(request->pathArg(2));

    if (pin < 0 || type == INVALID || value < 0) {
        return request->send(400);
    }

    if (type == LINEAR) {
        analogWrite(pin, value);
    } else if (type == DIGITAL) {
        digitalWrite(pin, (uint8_t) value);
    }

    fs.write(base_path + pin + F("/value"), [&](File &f) { f.print(value); });
    fs.write(base_path + pin + F("/type"), [&](File &f) { f.print(type); });
    request->send(200);
}

void ControllerGPIO::read(Request *request) const {
    const auto pin = get_digit(request->pathArg(0));
    const auto type = get_type(request->pathArg(1));

    if (pin < 0 || type == INVALID) {
        return request->send(400);
    }

    auto value = 0;
    if (type == LINEAR) {
        value = analogRead(pin);
    } else if (type == DIGITAL) {
        value = digitalRead(pin);
    }

    StaticJsonDocument<32> doc;
    doc[F("value")] = value;

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerGPIO::info(Request *request) const {
    auto pin = get_digit(request->pathArg(0));
    if (pin < 0) {
        return request->send(400);
    }

    StaticJsonDocument<128> doc;
    String path = base_path + pin + F("/");
    fs.ls(path, [&](const String &n, const bool dir) {
        if (dir) return;

        fs.read(path + F("/") + n, [&](File &f) {
            doc[n] = f.readString();
        });
    });

    String response;
    if (doc.isNull()) {
        request->send(404);
    } else {
        serializeJson(doc, response);
        request->send(200, mimeType(mime::json), response);
    }
}

void ControllerGPIO::list(Request *request) const {
    StaticJsonDocument<64> doc;

    fs.ls(base_path, [&](const String &n, const bool dir) {
        if (dir)
            doc.add(n.toInt());
    });

    String response;
    serializeJson(doc.isNull() ? doc.to<JsonArray>() : doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerGPIO::subscribe(EspServer &rest) {
    rest.on(HTTP_GET, Uri(F("/api/v1/gpio")),
            std::bind(&ControllerGPIO::list, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriBraces(F("/api/v1/gpio/{}")),
            std::bind(&ControllerGPIO::info, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/gpio/{}")),
            std::bind(&ControllerGPIO::toggle, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, UriBraces(F("/api/v1/gpio/{}")),
            std::bind(&ControllerGPIO::rm, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriBraces(F("/api/v1/gpio/{}/{}")),
            std::bind(&ControllerGPIO::read, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/gpio/{}/{}/{}")),
            std::bind(&ControllerGPIO::write, this, std::placeholders::_1));
    rest.on(HTTP_POST, UriBraces(F("/api/v1/gpio/{}/{}")),
            std::bind(&ControllerGPIO::mode, this, std::placeholders::_1));
}

void ControllerGPIO::subscribe(EspWebSocket &ws) {
    ws.on(HTTP_GET, Uri(F("/api/v1/gpio")),
          std::bind(&ControllerGPIO::list, this, std::placeholders::_1));
    ws.on(HTTP_GET, UriBraces(F("/api/v1/gpio/{}")),
          std::bind(&ControllerGPIO::info, this, std::placeholders::_1));
    ws.on(HTTP_PUT, UriBraces(F("/api/v1/gpio/{}")),
          std::bind(&ControllerGPIO::toggle, this, std::placeholders::_1));
    ws.on(HTTP_DELETE, UriBraces(F("/api/v1/gpio/{}")),
          std::bind(&ControllerGPIO::rm, this, std::placeholders::_1));
    ws.on(HTTP_GET, UriBraces(F("/api/v1/gpio/{}/{}")),
          std::bind(&ControllerGPIO::read, this, std::placeholders::_1));
    ws.on(HTTP_PUT, UriBraces(F("/api/v1/gpio/{}/{}/{}")),
          std::bind(&ControllerGPIO::write, this, std::placeholders::_1));
    ws.on(HTTP_POST, UriBraces(F("/api/v1/gpio/{}/{}")),
          std::bind(&ControllerGPIO::mode, this, std::placeholders::_1));
}
