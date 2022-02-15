#pragma once

#include <commons.h>
#include <Pixels.h>
#include <FileSystem.h>

typedef std::function<void(uint8_t l, pixel::color c[])> ColorConsumer;

class PixelsRepository {
protected:
    const static String base;
private:
    const String base_path;
    FileSystem &fs;
public:
    explicit PixelsRepository(FileSystem &fs) : base_path(base + "/"), fs(fs) {}
    explicit PixelsRepository(FileSystem &fs, uint8_t i) : base_path(base + "-" + i + "/"), fs(fs) {}

    uint8_t get_length(uint8_t fallback = 0, bool init = false);
    void set_length(uint8_t l);

    pixel::state get_state() const;
    void set_state(pixel::state s);

    pixel::color get_color() const;
    void set_color(pixel::color color);

    uint8_t get_brightness() const;
    void set_brightness(uint8_t color);

    pixel::mode get_mode() const;
    void set_mode(pixel::mode t);

    void set_params(pixel::params p);
    pixel::params get_params() const;

    void get_colors(ColorConsumer subscriber) const;
    void set_colors(uint8_t len, pixel::color colors[]);

    void configure(Pixels &pixels);
};

