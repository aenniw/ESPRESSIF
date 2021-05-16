#pragma once

#include <commons.h>
#include <Pixels.h>
#include <FileSystem.h>

using animation::type;

typedef std::function<void(const uint32_t c)> ColorConsumer;

class PixelsRepository {
private:
    const String base_path;
    FileSystem &fs;
public:
    explicit PixelsRepository(FileSystem &fs) : base_path(F("/pixels/")), fs(fs) {}

    uint32_t get_color() const;
    void set_color(const HtmlColor &color);

    type get_animation_type() const;
    void set_animation_type(type t);

    void set_animation_duration(uint16_t d);
    uint16_t get_animation_duration() const;

    void get_colors(ColorConsumer subscriber) const;
    void add_color(HtmlColor color);
    void rm_color(uint8_t pixel);
};

