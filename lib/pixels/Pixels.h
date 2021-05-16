#pragma once

#include <commons.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>

namespace pixel {
    typedef enum {
        STATIC,
        FADE,
        RAINBOW,
        TRANSITION
    } mode;

    typedef enum {
        OFF, ON
    } state;

    typedef struct {
        uint16_t hue: 9;
        uint16_t sat: 7;
    } color;

    typedef struct {
        uint32_t duration: 16;
        uint32_t chained: 1;
        uint32_t randomized: 1;
    } params;

    typedef struct {
        uint16_t len;
        uint64_t mask;
        uint16_t end;
        uint16_t start;
    } animator_params;

    typedef std::function<HsbColor(uint16_t i, float p)> ColorSupplier;

    uint8_t power_scale(uint16_t mA, uint8_t V, uint16_t len);
}

class Pixels : public Service {
private:
    NeoPixelAnimator animator;
    AnimUpdateCallback animations[pixel::mode::TRANSITION + 1];
    pixel::color color = { 0u, 0u };
    pixel::state state = pixel::OFF;
    std::vector<pixel::color> animation_colors;
    bool chained = false, randomized = false;
protected:
    virtual uint16_t length() const = 0;
    virtual void set_pixels(const HsbColor &c, bool correct) = 0;
    virtual RgbColor get_pixel(uint16_t i) = 0;
    virtual void set_pixel(uint16_t i, const HsbColor &c, bool correct) = 0;
    virtual void show() = 0;

    void animate(const AnimationParam &param,
                 const pixel::ColorSupplier &c,
                 const pixel::ColorSupplier &s,
                 bool dim);
    pixel::animator_params refresh(const AnimationParam &param);
    void fade(const AnimationParam &param);
    void rainbow(const AnimationParam &param);
    void transition(const AnimationParam &param);
public:
    explicit Pixels(uint16_t len);
    void cycle() override;

    pixel::state get_state() const;
    void set_state(pixel::state s);

    void set_color(pixel::color c);
    void set_color(pixel::color c, bool refresh);
    pixel::color get_color() const;

    virtual void set_brightness(uint8_t b) = 0;
    virtual uint8_t get_brightness() const = 0;

    pixel::mode get_mode() const;
    void set_mode(pixel::mode m, pixel::params p);

    void set_colors(uint8_t l, pixel::color colors[]);
};


template<typename T_COLOR_FEATURE, typename T_METHOD, typename T_GAMMA>
class NeoPixels : private NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>, public Pixels {
private:
    NeoGamma<T_GAMMA> gamma;
    uint8_t power_cap = 255;
protected:
    void set_pixels(const HsbColor &c, const bool correct) override {
        if (correct) {
            this->ClearTo(gamma.Correct(RgbColor(c)));
        } else {
            this->ClearTo(c);
        }
    }
    void set_pixel(uint16_t i, const HsbColor &c, const bool correct) override {
        if (correct) {
            this->SetPixelColor(i, gamma.Correct(RgbColor(c)));
        } else {
            this->SetPixelColor(i, c);
        }
    }
    RgbColor get_pixel(uint16_t i) override { return this->GetPixelColor(i); }
    void show() override {
        if (this->CanShow()) {
            this->Show();
        }
    }
    void begin() override { this->Begin(); }
    uint16_t length() const override { return this->PixelCount(); }

public:
    NeoPixels(uint16_t len, uint8_t pin, uint16_t mA = 1000, uint8_t V = 5) :
            NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>(len, pin), Pixels(len) {
        this->power_cap = pixel::power_scale(mA, V, len);
    }
    void set_brightness(uint8_t b) override { this->SetBrightness(map(b, 0, 255, 0, power_cap)); };
    uint8_t get_brightness() const override { return map(this->GetBrightness(), 0, power_cap, 0, 255); };
};