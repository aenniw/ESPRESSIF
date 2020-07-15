#include <ArduinoJson.h>
#include <EspServer.h>
#ifdef ARDUINO_ARCH_ESP32
#include <analogWrite.h>
#include <detail/mimetable.h>
#endif
#include "ControllerGPIO.h"

void ControllerGPIO::begin() {
    fs.ls(base_path, [this](const String &n, const bool dir) {
        if (!dir) return;

        auto pin = (uint8_t) n.toInt();
        fs.read(base_path + n + F("/type"), [this, &pin, &n](File &t) {
            const auto gpio_type = (GPIO_TYPE) t.readString().toInt();
            fs.read(base_path + n + F("/value"), [&](File &v) {
                switch (gpio_type) {
                    case ANAL:
                        analogWrite(pin, v.readString().toInt());
                        break;
                    case DIGI:
                        digitalWrite(pin, (uint8_t) v.readString().toInt());
                        break;
                }
            });
        });

        fs.read(base_path + n + F("/mode"), [&](File &m) {
            const uint8_t mode = (uint8_t) m.readString().toInt();
            pinMode(pin, mode);
            LOG("pinMode %d %s", pin, mode);
        });
    });
}

void ControllerGPIO::delete_gpio(Request *request) const {
    request->send(fs.rm(base_path + request->pathArg(0)) ? 200 : 404);
}

void ControllerGPIO::set_mode(Request *request) const {
    const auto pin = (uint8_t) request->pathArg(0).toInt();
    const auto mode = (GPIO_MODE) (request->pathArg(1).length() - 5);
    pinMode(pin, mode);
    fs.write(base_path + pin + F("/mode"), [&](File &f) { f.print(mode); });
    request->send(200);
}

void ControllerGPIO::toggle(Request *request) const {
    const auto pin = (uint8_t) request->pathArg(0).toInt();
    const auto value = (uint8_t) !digitalRead(pin);
    digitalWrite(pin, value);
    fs.write(base_path + pin + F("/value"), [&](File &f) { f.print(value); });
    fs.write(base_path + pin + F("/type"), [&](File &f) { f.print(DIGI); });
    request->send(200);
}

void ControllerGPIO::write(Request *request) const {
    const auto pin = (uint8_t) request->pathArg(0).toInt();
    const auto type = (GPIO_TYPE) request->pathArg(1).length();
    const auto value = request->pathArg(2).toInt();
    if (type == ANAL) {
        analogWrite(pin, value);
    } else if (type == DIGI) {
        digitalWrite(pin, (uint8_t) value);
    } else {
        return request->send(400);
    }
    fs.write(base_path + pin + F("/value"), [&](File &f) { f.print(value); });
    fs.write(base_path + pin + F("/type"), [&](File &f) { f.print(type); });
    request->send(200);
}

void ControllerGPIO::read(Request *request) const {
    const auto pin = (uint8_t) request->pathArg(0).toInt();
    const auto mode = (GPIO_TYPE) request->pathArg(1).length();
    auto value = 0;
    if (mode == ANAL) {
        value = analogRead(pin);
    } else if (mode == DIGI) {
        value = digitalRead(pin);
    } else {
        return request->send(400);
    }

    StaticJsonDocument<32> doc;
    doc[F("value")] = value;

    String response;
    serializeJson(doc, response);
    request->send(200, mineType(mime::json), response);
}

void ControllerGPIO::info(Request *request) const {
    StaticJsonDocument<128> doc;
    String path = base_path + request->pathArg(0) + F("/");

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
        request->send(200, mineType(mime::json), response);
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
    request->send(200, mineType(mime::json), response);
}

void ControllerGPIO::subscribe(EspServer &rest) const {
    rest.on(HTTP_GET, F(R"(^\/api\/v1\/gpio$)"),
            std::bind(&ControllerGPIO::list, this, std::placeholders::_1));
    rest.on(HTTP_GET, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})$)"),
            std::bind(&ControllerGPIO::info, this, std::placeholders::_1));
    rest.on(HTTP_PUT, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})$)"),
            std::bind(&ControllerGPIO::toggle, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})$)"),
            std::bind(&ControllerGPIO::delete_gpio, this, std::placeholders::_1));
    rest.on(HTTP_GET, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})\/(analog|digital)$)"),
            std::bind(&ControllerGPIO::read, this, std::placeholders::_1));
    rest.on(HTTP_PUT, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})\/(analog|digital)\/([0-9]{1,4})$)"),
            std::bind(&ControllerGPIO::write, this, std::placeholders::_1));
    rest.on(HTTP_POST, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})\/(input|output)$)"),
            std::bind(&ControllerGPIO::set_mode, this, std::placeholders::_1));
}
