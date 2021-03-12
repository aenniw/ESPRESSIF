#include "Pixels.h"
#include <cmath>

using animation::type;

HsbColor Pixels::get_color() const {
    return color;
}

void Pixels::set_color(HtmlColor c, const bool refresh) {
    set_color(HsbColor(RgbColor(c)), refresh);
}

void Pixels::set_color(HsbColor c, const bool refresh) {
    LOG("color %d %d %d", c.H, c.S, c.B);
    color = c;
    if (refresh) {
        animator.StopAll();
        set_pixels(color);
    }
}

bool Pixels::set_animation(type t, uint16_t d) {
    if (d < 200 || t < 0 || t >= type::NONE) {
        return false;
    }
    if (animator.IsAnimationActive(t)) {
        LOG("scale animation %d %d", t, d);
        animator.ChangeAnimationDuration(t, d);
        return true;
    }

    LOG("set animation %d %d", t, d);
    animator.StopAll();
    animator.StartAnimation(t, d, animations[t]);
    return true;
}

type Pixels::animation() const {
    for (uint8_t t = 0; t < type::NONE; t++) {
        if (animator.IsAnimationActive(t)) {
            return (type) t;
        }
    }
    return type::NONE;
}

bool Pixels::animating() const { return animator.IsAnimating(); }

bool Pixels::add_color(HtmlColor c) {
    if (animation_colors.size() >= MAX_COLORS) {
        return false;
    }
    LOG("add anim color %d", c.Color);
    animation_colors.push_back(c.Color);
    return true;
}

bool Pixels::rm_color(uint8_t i) {
    if (i >= animation_colors.size()) {
        return false;
    }

    LOG("remove anim color %d", i);
    animation_colors.erase(animation_colors.begin() + i);
    return true;
}

Pixels::Pixels(uint16_t len) : color(0, 0, 0), animator(len, NEO_CENTISECONDS) {
    animations[type::ANIMATION_0] = std::bind(&Pixels::fade, this, std::placeholders::_1);
    animations[type::ANIMATION_1] = std::bind(&Pixels::fade_chain, this, std::placeholders::_1);
    animations[type::ANIMATION_2] = std::bind(&Pixels::rainbow, this, std::placeholders::_1);
    animations[type::ANIMATION_3] = std::bind(&Pixels::rainbow_chain, this, std::placeholders::_1);
    animations[type::ANIMATION_4] = std::bind(&Pixels::transition, this, std::placeholders::_1);
    animations[type::ANIMATION_5] = std::bind(&Pixels::transition_chain, this, std::placeholders::_1);
}

void Pixels::cycle() {
    if (animator.IsAnimating()) {
        animator.UpdateAnimations();
    }
    show();
}

void Pixels::refresh(const AnimationParam &param) {
    if (param.state == AnimationState_Completed) {
        animator.RestartAnimation(param.index);
    }
}

static float fade_brightness(const float b, const float p) {
    return std::max(b, 0.025f) * (float) (1.0f - std::abs(std::sin(PI * p - PI)));
}

static float progress_offset(const uint16_t l, const uint16_t i, float p) {
    p = p + ((float) i / (float) l);
    return p >= 1.0f ? p - 1.0f : p;
}

void Pixels::fade(const AnimationParam &param) {
    set_pixels(HsbColor(color.H, color.S, fade_brightness(color.B, param.progress)));
    refresh(param);
}

void Pixels::fade_chain(const AnimationParam &param) {
    const auto len = length();
    for (uint16_t i = 0; i < len; i++) {
        set_pixel(i, HsbColor(
                color.H, color.S,
                fade_brightness(color.B, progress_offset(len, i, param.progress))
        ));
    }
    refresh(param);
}

void Pixels::rainbow(const AnimationParam &param) {
    set_pixels(HsbColor(param.progress, 1, color.B));
    refresh(param);
}

void Pixels::rainbow_chain(const AnimationParam &param) {
    const auto len = length();
    for (uint16_t i = 0; i < len; i++) {
        set_pixel(i, HsbColor(
                progress_offset(len, i, param.progress), 1, color.B)
        );
    }
    refresh(param);
}

static HsbColor color_offset(const std::vector<uint32_t> &colors, const double b, const float p, const uint16_t i = 0) {
    const auto color = colors[
            ((uint16_t) std::floor(colors.size() * p) + i) % colors.size()
    ];
    return HsbColor{RgbColor(
            ((color & 0xff0000u) >> 16u) * b,
            ((color & 0x00ff00u) >> 8u) * b,
            (color & 0x0000ffu) * b
    )};
}

void Pixels::transition(const AnimationParam &param) {
    if (!animation_colors.empty()) {
        set_pixels(color_offset(animation_colors, color.B, param.progress));
    }
    refresh(param);
}

void Pixels::transition_chain(const AnimationParam &param) {
    if (!animation_colors.empty()) {
        const auto len = length();
        for (uint16_t i = 0; i < len; i++) {
            set_pixel(i, color_offset(animation_colors, color.B, param.progress, i));
        }
    }
    refresh(param);
}


