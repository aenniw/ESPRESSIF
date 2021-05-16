#pragma once

#include <commons.h>
#include <Pixels.h>
#include <EspServer.h>
#include <EspWebSocket.h>
#include <PixelsRepository.h>

class ControllerPixels : public Subscriber<EspServer>, public Subscriber<EspWebSocket>, public Service {
private:
    Pixels &pixels;
    PixelsRepository repository;
private:
    static bool parse_color(HtmlColor &hexColor, const String &c);

protected:
    void color(Request *request) const;
    void set_color(Request *request);
    void brightness(Request *request) const;
    void set_brightness(Request *request);
    void animation(Request *request) const;
    void set_animation(Request *request);
    void colors(Request *request) const;
    void add_colors(Request *request);
    void rm_colors(Request *request);

public:
    explicit ControllerPixels(FileSystem &fs, Pixels &pixels) :
            pixels(pixels), repository(fs) {}

    void begin() override;
    void subscribe(EspServer &rest) override;
    void subscribe(EspWebSocket &ws) override;
    void cycle() override {};
};