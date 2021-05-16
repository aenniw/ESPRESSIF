#include "ControllerPixels.h"
#include <uri/UriBraces.h>
#include <ArduinoJson.h>
#ifdef ARDUINO_ARCH_ESP32
#include <detail/mimetable.h>
#endif

void ControllerPixels::state(RestRequest *request) const {
    StaticJsonDocument<64> doc;
    doc[F("state")] = (uint8_t) pixels.get_state();

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::set_state(RestRequest *request) {
    pixel::state state = pixels.get_state() == pixel::OFF ? pixel::ON : pixel::OFF;

    const auto arg = get_digit(request->pathArg(0));
    if (arg >= 0) {
        state = (pixel::state) arg;
    }

    pixels.set_state(state);
    request->send(200);
}

void ControllerPixels::color(RestRequest *request) const {
    StaticJsonDocument<48> doc;
    doc[F("hue")] = pixels.get_color().hue;
    doc[F("sat")] = pixels.get_color().sat;

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::set_color(RestRequest *request) {
    const auto arg = F("plain"), hue = F("hue"), sat = F("sat");

    StaticJsonDocument<128> doc;
    if (request->hasArg(arg) &&
        !deserializeJson(doc, request->arg(arg)) &&
        !doc[hue].isNull() &&
        !doc[sat].isNull()
            ) {
        const pixel::color color = {
                .hue = doc[hue].as<uint16_t>(),
                .sat = doc[sat].as<uint8_t>()
        };
        pixels.set_color(color);
        repository.set_color(color);
        request->send(200);
    } else {
        request->send(400);
    }
}

void ControllerPixels::brightness(RestRequest *request) const {
    StaticJsonDocument<64> doc;
    doc[F("brightness")] = pixels.get_brightness();

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);

}

void ControllerPixels::set_brightness(RestRequest *request) {
    const auto b = get_digit(request->pathArg(0));
    if (b >= 0 && b <= 255) {
        pixels.set_brightness(b);
        repository.set_brightness(b);
        request->send(200);
    } else {
        request->send(400);
    }
}

void ControllerPixels::mode(RestRequest *request) const {
    StaticJsonDocument<128> doc;
    const auto params = repository.get_params();
    doc[F("mode")] = (uint8_t) repository.get_mode();
    doc[F("duration")] = params.duration;
    doc[F("randomized")] = params.randomized;
    doc[F("chained")] = params.chained;

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::set_mode(RestRequest *request) {
    const auto arg = F("plain"),
            mode = F("mode"),
            duration = F("duration"),
            randomized = F("randomized"),
            chained = F("chained");

    StaticJsonDocument<256> doc;
    if (request->hasArg(arg) &&
        !deserializeJson(doc, request->arg(arg)) &&
        !doc[mode].isNull() &&
        !doc[duration].isNull() &&
        !doc[chained].isNull() &&
        !doc[randomized].isNull()
            ) {
        pixel::mode m = doc[mode].as<pixel::mode>();
        pixel::params p = {
                doc[duration].as<uint16_t>(),
                doc[chained].as<bool>(),
                doc[randomized].as<bool>(),
        };
        pixels.set_mode(m, p);
        repository.set_mode(m);
        repository.set_params(p);
        request->send(200);
    } else {
        request->send(400);
    }
}

void ControllerPixels::colors(RestRequest *request) const {
    StaticJsonDocument<768> doc;

    repository.get_colors([&](uint8_t l, const pixel::color colors[]) {
        for (uint8_t i = 0; i < l; i++) {
            JsonObject cDoc = doc.createNestedObject();
            cDoc[F("hue")] = colors[i].hue;
            cDoc[F("sat")] = colors[i].sat;
        }
    });

    String response;
    serializeJson(doc.isNull() ? doc.to<JsonArray>() : doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::set_colors(RestRequest *request) {
    const auto arg = F("plain"), hue = F("hue"), sat = F("sat");

    StaticJsonDocument<768> doc;
    if (request->hasArg(arg) && !deserializeJson(doc, request->arg(arg))) {
        auto colorsJson = doc.as<JsonArray>();
        pixel::color colors[colorsJson.size()];

        for (size_t i = 0; i < colorsJson.size(); i++) {
            JsonVariant colorJson = colorsJson[i];
            colors[i] = {
                    .hue = colorJson[hue].as<uint16_t>(),
                    .sat = colorJson[sat].as<uint8_t>()
            };
        }

        pixels.set_colors(colorsJson.size(), colors);
        repository.set_colors(colorsJson.size(), colors);
        request->send(200);
    } else {
        request->send(400);
    }
}

void ControllerPixels::begin() {
    repository.configure(pixels);
}

void ControllerPixels::subscribe(EspServer &rest) {
    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/state")),
            std::bind(&ControllerPixels::state, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/state/{}")),
            std::bind(&ControllerPixels::set_state, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/color")),
            std::bind(&ControllerPixels::color, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/color")),
            std::bind(&ControllerPixels::set_color, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/brightness")),
            std::bind(&ControllerPixels::brightness, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/brightness/{}")),
            std::bind(&ControllerPixels::set_brightness, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/mode")),
            std::bind(&ControllerPixels::mode, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/mode")),
            std::bind(&ControllerPixels::set_mode, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/colors")),
            std::bind(&ControllerPixels::colors, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/colors")),
            std::bind(&ControllerPixels::set_colors, this, std::placeholders::_1));
}