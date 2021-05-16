#include "ControllerPixels.h"
#include <uri/UriBraces.h>
#include <ArduinoJson.h>
#ifdef ARDUINO_ARCH_ESP32
#include <detail/mimetable.h>
#endif

using animation::type;

void ControllerPixels::begin() {
    pixels.set_color(HtmlColor(repository.get_color()), false);
    repository.get_colors([&](const uint32_t color) {
        pixels.add_color(HtmlColor(color));
    });
    pixels.set_animation(
            repository.get_animation_type(),
            repository.get_animation_duration()
    );

    if (pixels.animation() == type::NONE) {
        pixels.set_color(pixels.get_color());
    }
}

void ControllerPixels::set_color(Request *request) {
    auto color = HtmlColor();
    if (!parse_color(color, request->pathArg(0))) {
        return request->send(400);
    }

    pixels.set_color(color);
    repository.set_color(color);
    repository.set_animation_type(type::NONE);
    request->send(200);
}

void ControllerPixels::color(Request *request) const {
    if (pixels.animating()) {
        return request->send(404);
    }
    StaticJsonDocument<64> doc;
    char color[9] = "0x000000";
    snprintf(color, 9, "0x%06X", repository.get_color());
    doc[F("color")] = color;

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::set_brightness(Request *request) {
    const auto brightness = get_float((request->pathArg(0)));
    if (!pixels.set_brightness(brightness)) {
        return request->send(400);
    }

    repository.set_color(HtmlColor(RgbColor(pixels.get_color())));
    request->send(200);
}

void ControllerPixels::brightness(Request *request) const {
    StaticJsonDocument<32> doc;
    doc[F("brightness")] = pixels.get_brightness();

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::set_animation(Request *request) {
    const type t = (type) get_digit((request->pathArg(0)));
    const uint16_t d = get_digit((request->pathArg(1)));
    if (!pixels.set_animation(t, d)) {
        return request->send(400);
    }

    repository.set_animation_type(t);
    repository.set_animation_duration(d);
    request->send(200);
}

void ControllerPixels::animation(Request *request) const {
    StaticJsonDocument<128> doc;

    doc[F("type")] = repository.get_animation_type();
    doc[F("duration")] = repository.get_animation_duration();

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::colors(Request *request) const {
    StaticJsonDocument<256> doc;

    repository.get_colors([&](const uint32_t c) {
        doc.add(c);
    });

    String response;
    serializeJson(doc.isNull() ? doc.to<JsonArray>() : doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::add_colors(Request *request) {
    auto color = HtmlColor();
    if (!parse_color(color, request->pathArg(0)) ||
        !pixels.add_color(color)) {
        return request->send(400);
    }

    repository.add_color(color);
    request->send(200);
}

void ControllerPixels::rm_colors(Request *request) {
    auto pixel = get_digit((request->pathArg(0)));
    if (pixel < 0 || !pixels.rm_color(pixel)) {
        return request->send(400);
    }

    repository.rm_color(pixel);
    request->send(200);
}

void ControllerPixels::subscribe(EspServer &rest) {
    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/color")),
            std::bind(&ControllerPixels::color, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/color/{}")),
            std::bind(&ControllerPixels::set_color, this, std::placeholders::_1));
    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/brightness")),
            std::bind(&ControllerPixels::brightness, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/brightness/{}")),
            std::bind(&ControllerPixels::set_brightness, this, std::placeholders::_1));

    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/animation")),
            std::bind(&ControllerPixels::animation, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/animation/{}/{}")),
            std::bind(&ControllerPixels::set_animation, this, std::placeholders::_1));

    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/colors")),
            std::bind(&ControllerPixels::colors, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/colors/{}")),
            std::bind(&ControllerPixels::add_colors, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, UriBraces(F("/api/v1/pixels/colors/{}")),
            std::bind(&ControllerPixels::rm_colors, this, std::placeholders::_1));
}

void ControllerPixels::subscribe(EspWebSocket &ws) {
    ws.on(HTTP_GET, UriBraces(F("/api/v1/pixels/color")),
          std::bind(&ControllerPixels::color, this, std::placeholders::_1));
    ws.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/color/{}")),
          std::bind(&ControllerPixels::set_color, this, std::placeholders::_1));
    ws.on(HTTP_GET, UriBraces(F("/api/v1/pixels/brightness")),
          std::bind(&ControllerPixels::brightness, this, std::placeholders::_1));
    ws.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/brightness/{}")),
          std::bind(&ControllerPixels::set_brightness, this, std::placeholders::_1));

    ws.on(HTTP_GET, UriBraces(F("/api/v1/pixels/animation")),
          std::bind(&ControllerPixels::animation, this, std::placeholders::_1));
    ws.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/animation/{}/{}")),
          std::bind(&ControllerPixels::set_animation, this, std::placeholders::_1));

    ws.on(HTTP_GET, UriBraces(F("/api/v1/pixels/colors")),
          std::bind(&ControllerPixels::colors, this, std::placeholders::_1));
    ws.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/colors/{}")),
          std::bind(&ControllerPixels::add_colors, this, std::placeholders::_1));
    ws.on(HTTP_DELETE, UriBraces(F("/api/v1/pixels/colors/{}")),
          std::bind(&ControllerPixels::rm_colors, this, std::placeholders::_1));
}

bool ControllerPixels::parse_color(HtmlColor &hexColor, const String &c) {
    return hexColor.Parse<HtmlShortColorNames>(c) > 0 ||
           hexColor.Parse<HtmlShortColorNames>("#" + c) > 0;
}
