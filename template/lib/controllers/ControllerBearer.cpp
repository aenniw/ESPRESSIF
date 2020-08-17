#include "ControllerBearer.h"
#include <ArduinoJson.h>
#ifdef ARDUINO_ARCH_ESP32
#include <MD5Builder.h>
#include <detail/mimetable.h>
#define RNG esp_random()
#else
#define RNG RANDOM_REG32
#endif

String ControllerBearer::generate_token() {
    MD5Builder md5_segment{};
    md5_segment.begin();
    for (uint8_t i = 0; i < 8; i++) {
        uint32_t rnd = RNG;
        uint8_t nonceBuffer[4];
        memcpy(nonceBuffer, &rnd, 4);
        md5_segment.add(nonceBuffer, 4);
    }

    md5_segment.calculate();
    String token = md5_segment.toString();
    if (fs.write(base_path + F("tokens"), [&](File &f) {
        f.print(token);
        f.print('\n');
    }, "a")) {
        LOG("Added token %s", token.c_str());
        canGenerate = false;
        return token;
    }

    return F("");
}

void ControllerBearer::add(Request *request) {
    if (!canGenerate)
        return request->send(401);

    StaticJsonDocument<64> token;
    token[F("token")] = generate_token();

    String response;
    serializeJson(token, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerBearer::list(Request *request) const {
    StaticJsonDocument<512> tokens;

    fs.read(base_path + F("tokens"), [&](File &f) {
        while (f.available()) {
            tokens.add(f.readStringUntil('\n'));
        }
    });

    if (tokens.isNull()) {
        return request->send(404);
    }

    String response;
    serializeJson(tokens, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerBearer::reset(Request *request) const {
    if (fs.rm(base_path)) {
        return request->send(200);
    }
    request->send(404);
}

void ControllerBearer::enable() {
    elapsed = 0;
    canGenerate = true;
    LOG("Token creation enabled");
}

bool ControllerBearer::test(String &t) const {
    if (t.length() != 32) return false;
    bool matches = false;
    fs.read(base_path + F("tokens"), [&](File &f) {
        while (f.available() && !matches) {
            matches = f.readStringUntil('\n').equals(t);
        }
    });
    return matches;
}

void ControllerBearer::subscribe(EspServer &rest) {
    rest.on(HTTP_GET, Uri(F("/api/v1/bearer/")),
            std::bind(&ControllerBearer::list, this, std::placeholders::_1));
    rest.on(HTTP_POST, Uri(F("/api/v1/bearer/")),
            std::bind(&ControllerBearer::add, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, Uri(F("/api/v1/bearer/")),
            std::bind(&ControllerBearer::reset, this, std::placeholders::_1));
}

void ControllerBearer::begin() { elapsed = 0; }

void ControllerBearer::cycle() {
    if (elapsed > tokenGenerateTimeout) {
        if (canGenerate) {
            canGenerate = false;
            LOG("Token creation disabled");
        }
        elapsed = 0;
    }
}
