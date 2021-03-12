#pragma once

#include <commons.h>
#include <Pixels.h>
#include <EspServer.h>
#include <EspWebSocket.h>
#include <FileSystem.h>

class ControllerPixels : public Subscriber<EspServer>, public Subscriber<EspWebSocket>, public Service {
private:
    const String base_path;
    FileSystem &fs;
    Pixels &pixels;
private:
    static bool parse_color(HtmlColor &hexColor, const String &c);

protected:
    void color(Request *request) const;
    void set_color(Request *request);
    void set_brightness(Request *request);
    void animation(Request *request) const;
    void set_animation(Request *request);
    void colors(Request *color) const;
    void add_colors(Request *request);
    void rm_colors(Request *color);

public:
    explicit ControllerPixels(FileSystem &fs, Pixels &pixels) :
            base_path(F("/pixels/")), fs(fs), pixels(pixels) {}

    void begin() override;
    void subscribe(EspServer &rest) override;
    void subscribe(EspWebSocket &ws) override;
    void cycle() override {};
};