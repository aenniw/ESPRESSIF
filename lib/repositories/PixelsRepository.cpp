#include "PixelsRepository.h"

const String PixelsRepository::base = "/pixels";

uint8_t PixelsRepository::get_length(const uint8_t fallback, const bool init) {
    uint8_t l = fallback;
    auto exists = fs.read(base_path + F("length"), [&](File &f) {
        l = f.read();
    });
    if (!exists && init) {
        this->set_length(fallback);
    }
    return l;
}

void PixelsRepository::set_length(const uint8_t l) {
    fs.write(base_path + F("length"), [&](File &f) {
        f.write(l);
    });
}

pixel::state PixelsRepository::get_state() const {
    uint8_t s = pixel::state::OFF;
    fs.read(base_path + F("state"), [&](File &f) {
        s = f.read();
    });
    return (pixel::state) s;
}

void PixelsRepository::set_state(const pixel::state s) {
    fs.write(base_path + F("state"), [&](File &f) {
        f.write((uint8_t) s);
    });
}

pixel::color PixelsRepository::get_color() const {
    pixel::color color = {0u, 0u};
    fs.read(base_path + F("color"), [&](File &f) {
        f.read(reinterpret_cast<uint8_t *>(&color), sizeof(pixel::color));
    });
    return color;
}

void PixelsRepository::set_color(pixel::color color) {
    fs.write(base_path + F("color"), [&](File &f) {
        f.write(reinterpret_cast<uint8_t *>(&color), sizeof(pixel::color));
    });
}

uint8_t PixelsRepository::get_brightness() const {
    uint8_t brightness = 50u;
    fs.read(base_path + F("brightness"), [&](File &f) {
        brightness = f.read();
    });
    return brightness;
}

void PixelsRepository::set_brightness(const uint8_t brightness) {
    fs.write(base_path + F("brightness"), [&](File &f) {
        f.write(brightness);
    });
}

pixel::mode PixelsRepository::get_mode() const {
    auto m = pixel::mode::STATIC;
    fs.read(base_path + F("mode"), [&](File &f) {
        m = (pixel::mode) f.read();
    });
    return m;
}

void PixelsRepository::set_mode(pixel::mode t) {
    fs.write(base_path + F("mode"), [&](File &f) {
        f.write((uint8_t) t);
    });
}

pixel::params PixelsRepository::get_params() const {
    pixel::params p = {0u, false, false};
    fs.read(base_path + F("params"), [&](File &f) {
        f.read(reinterpret_cast<uint8_t *>(&p), sizeof(pixel::params));
    });
    return p;
}

void PixelsRepository::set_params(pixel::params p) {
    fs.write(base_path + F("params"), [&](File &f) {
        f.write(reinterpret_cast<uint8_t *>(&p), sizeof(pixel::params));
    });
}

void PixelsRepository::get_colors(ColorConsumer subscriber) const {
    if (!fs.read(base_path + F("colors"), [&](File &file) {
        uint8_t len = file.size();
        uint8_t data[len];
        file.read(data, len);
        subscriber(
                len / sizeof(pixel::color),
                reinterpret_cast<pixel::color *>(data)
        );
    })) {
        pixel::color data[0];
        subscriber(0, data);
    }
}

void PixelsRepository::set_colors(const uint8_t len, pixel::color colors[]) {
    fs.write(base_path + F("colors"), [&](File &file) {
        file.write(reinterpret_cast<uint8_t *>(colors), len * sizeof(pixel::color));
    });
}

void PixelsRepository::configure(Pixels &pixels) {
    pixels.set_color(get_color());
    pixels.set_brightness(get_brightness());
    get_colors([&](uint8_t len, pixel::color colors[]) {
        pixels.set_colors(len, colors);
    });
    pixels.set_mode(get_mode(), get_params());
    pixels.set_state(get_state());
}
