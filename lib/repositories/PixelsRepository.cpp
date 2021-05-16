#include "PixelsRepository.h"

uint32_t PixelsRepository::get_color() const {
    uint32_t color = 0;
    fs.read(base_path + F("/color"), [&](File &f) {
        color = f.read();
    });
    return color;
}

void PixelsRepository::set_color(const HtmlColor &color) {
    fs.write(base_path + F("/color"), [&](File &f) {
        f.print(color.Color);
    });
}

type PixelsRepository::get_animation_type() const {
    auto animation = type::NONE;
    fs.read(base_path + F("/animation/type"), [&](File &f) {
        animation = (type) f.read();
    });
    return animation;
}

void PixelsRepository::set_animation_duration(const uint16_t d) {
    fs.write(base_path + F("/animation/duration"), [&](File &f) {
        f.print(d);
    });
}

uint16_t PixelsRepository::get_animation_duration() const {
    auto duration = 0;
    fs.read(base_path + F("/animation/duration"), [&](File &f) {
        duration = f.read();
    });
    return duration;
}

void PixelsRepository::set_animation_type(const type t) {
    fs.write(base_path + F("/animation/type"), [&](File &f) {
        f.print(t);
    });
}

void PixelsRepository::get_colors(ColorConsumer subscriber) const {
    fs.read(base_path + F("/colors"), [&](File &file) {
        uint8_t buffer[3];
        while (file.read(buffer, 3)) {
            auto color = (uint32_t) ((buffer[0] << 16u) + (buffer[1] << 8u) + buffer[2]);
            subscriber(color);
        }
    });
}

void PixelsRepository::add_color(const HtmlColor color) {
    fs.write(base_path + F("/colors"), [&](File &file) {
        file.write((uint8_t) ((color.Color & 0xff0000u) >> 16u));
        file.write((uint8_t) ((color.Color & 0x00ff00u) >> 8u));
        file.write((uint8_t) (color.Color & 0x0000ffu));
    }, "a");
}

void PixelsRepository::rm_color(uint8_t pixel) {
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
}
