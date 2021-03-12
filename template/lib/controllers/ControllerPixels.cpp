#include "ControllerPixels.h"
#include <uri/UriBraces.h>
#include <ArduinoJson.h>
#ifdef ARDUINO_ARCH_ESP32
#include <detail/mimetable.h>
#endif

using animation::type;

void ControllerPixels::begin() {
    fs.read(base_path + F("/color"), [&](File &f) {
        pixels.set_color(HtmlColor(f.parseInt()), false);
    });
    fs.read(base_path + F("/colors"), [&](File &file) {
        uint8_t buffer[3];
        while (file.read(buffer, 3)) {
            uint32_t color = (buffer[0] << 16u) + (buffer[1] << 8u) + buffer[2];
            pixels.add_color(HtmlColor(color));
        }
    });
    fs.read(base_path + F("/animation/type"), [&](File &t) {
        fs.read(base_path + F("/animation/duration"), [&](File &d) {
            pixels.set_animation((type) t.parseInt(), d.parseInt());
        });
    });

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
    fs.write(base_path + F("/color"), [&](File &f) { f.print(color.Color); });
    fs.write(base_path + F("/animation/type"), [&](File &f) { f.print(type::NONE); });
    request->send(200);
}

void ControllerPixels::set_brightness(Request *request) {
    const auto brightness = get_float((request->pathArg(0)));
    if (brightness < 0 || brightness > 1) {
        return request->send(400);
    }

    const auto color = pixels.get_color();
    pixels.set_color(
            HsbColor(color.H, color.S, brightness),
            pixels.animation() == type::NONE
    );
    fs.write(base_path + F("/color"), [&](File &f) {
        f.print(HtmlColor(RgbColor(pixels.get_color())).Color);
    });
    request->send(200);
}

void ControllerPixels::color(Request *request) const {
    if (pixels.animating()) {
        return request->send(404);
    }
    StaticJsonDocument<64> doc;
    char color[9] = "0x000000";
    fs.read(base_path + F("/color"), [&](File &f) {
        snprintf(color, 9, "0x%06X", f.read());
    });
    doc[F("color")] = color;

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

    fs.write(base_path + F("/animation/type"), [&](File &f) { f.print(t); });
    fs.write(base_path + F("/animation/duration"), [&](File &f) { f.print(d); });
    request->send(200);
}

void ControllerPixels::animation(Request *request) const {
    StaticJsonDocument<128> doc;
    auto animation = type::NONE;
    auto duration = 0;

    fs.read(base_path + F("/animation/type"), [&](File &f) { animation = (type) f.read(); });
    fs.read(base_path + F("/animation/duration"), [&](File &f) { duration = f.read(); });

    doc[F("type")] = animation;
    doc[F("duration")] = duration;

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerPixels::colors(Request *request) const {
    StaticJsonDocument<256> doc;

    fs.read(base_path + F("/colors"), [&](File &file) {
        uint8_t buffer[3];
        while (file.read(buffer, 3)) {
            doc.add((uint32_t) ((buffer[0] << 16u) + (buffer[1] << 8u) + buffer[2]));
        }
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

    fs.write(base_path + F("/colors"), [&](File &file) {
        file.write((uint8_t) ((color.Color & 0xff0000u) >> 16u));
        file.write((uint8_t) ((color.Color & 0x00ff00u) >> 8u));
        file.write((uint8_t) (color.Color & 0x0000ffu));
    }, "a");

    request->send(200);
}

void ControllerPixels::rm_colors(Request *request) {
    auto pixel = get_digit((request->pathArg(0)));
    if (pixel < 0 || !pixels.rm_color(pixel)) {
        return request->send(400);
    }

    std::vector<uint8_t> colors;
    fs.read(base_path + F("/colors"), [&](File &file) {
        uint8_t buffer[3];
        while (file.read(buffer, 3)) {
            if (pixel-- == 0) continue;
            colors.push_back(buffer[0]);
            colors.push_back(buffer[1]);
            colors.push_back(buffer[2]);
        }
    });
    fs.write(base_path + F("/colors"), [&](File &file) {
        for (auto color: colors) {
            file.write(color);
        }
    });

    request->send(200);
}

void ControllerPixels::subscribe(EspServer &rest) {
    rest.on(HTTP_GET, UriBraces(F("/api/v1/pixels/color")),
            std::bind(&ControllerPixels::color, this, std::placeholders::_1));
    rest.on(HTTP_PUT, UriBraces(F("/api/v1/pixels/color/{}")),
            std::bind(&ControllerPixels::set_color, this, std::placeholders::_1));
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
    return hexColor.Parse<HtmlColorNames>(c) > 0 ||
           hexColor.Parse<HtmlColorNames>("#" + c) > 0;
}
