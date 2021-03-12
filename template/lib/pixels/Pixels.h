#pragma once

#include <commons.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

#define MAX_COLORS 64

namespace animation {
    typedef enum {
        ANIMATION_0,
        ANIMATION_1,
        ANIMATION_2,
        ANIMATION_3,
        ANIMATION_4,
        ANIMATION_5,
        NONE
    } type;
}

class Pixels : public Service {
private:
    HsbColor color;
    NeoPixelAnimator animator;
    std::vector<uint32_t> animation_colors;
    AnimUpdateCallback animations[animation::type::NONE];
protected:
    virtual uint16_t length() const = 0;
    virtual void set_pixels(const HsbColor &c) = 0;
    virtual void set_pixel(uint16_t i, const HsbColor &c) = 0;
    virtual void show() = 0;

    void refresh(const AnimationParam &param);
    void fade(const AnimationParam &param);
    void fade_chain(const AnimationParam &param);
    void rainbow(const AnimationParam &param);
    void rainbow_chain(const AnimationParam &param);
    void transition(const AnimationParam &param);
    void transition_chain(const AnimationParam &param);
public:
    explicit Pixels(uint16_t len);
    void cycle() override;

    void set_color(HtmlColor c, bool refresh = true);
    void set_color(HsbColor c, bool refresh = true);
    HsbColor get_color() const;

    bool animating() const;
    bool set_animation(animation::type t, uint16_t d);
    animation::type animation() const;

    bool add_color(HtmlColor c);
    bool rm_color(uint8_t i);
};


template<typename T_COLOR_FEATURE, typename T_METHOD>
class NeoPixels : private NeoPixelBus<T_COLOR_FEATURE, T_METHOD>, public Pixels {
protected:
    uint16_t length() const override { return this->PixelCount(); }
    void set_pixels(const HsbColor &c) override { this->ClearTo(c); }
    void set_pixel(uint16_t i, const HsbColor &c) override { this->SetPixelColor(i, c); }
    void show() override { this->Show(); }
    void begin() override { this->Begin(); }

public:
    NeoPixels(uint16_t len, uint8_t pin) : NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(len, pin), Pixels(len) {}
};