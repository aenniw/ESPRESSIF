#pragma once

#include <commons.h>
#include <Pixels.h>
#include <EspServer.h>
#include <PixelsRepository.h>

class ControllerPixels : public Subscriber<EspServer>, public Service {
private:
    Pixels &pixels;
    PixelsRepository repository;
private:
protected:
    void state(RestRequest *request) const;
    void set_state(RestRequest *request);
    void color(RestRequest *request) const;
    void set_color(RestRequest *request);
    void brightness(RestRequest *request) const;
    void set_brightness(RestRequest *request);
    void mode(RestRequest *request) const;
    void set_mode(RestRequest *request);
    void colors(RestRequest *request) const;
    void set_colors(RestRequest *request);

public:
    explicit ControllerPixels(FileSystem &fs, Pixels &pixels) :
            pixels(pixels), repository(fs) {}

    void begin() override;
    void subscribe(EspServer &rest) override;
    void cycle() override {};
};