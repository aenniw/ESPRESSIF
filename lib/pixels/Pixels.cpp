#include "Pixels.h"

const static auto BLACK = RgbColor(0, 0, 0);

HsbColor pixel::to_color(const pixel::color c) {
    return {c.hue / 360.0f, c.sat / 100.0f, 1};
}

pixel::state Pixels::get_state() const {
    return state;
}

void Pixels::set_state(const pixel::state s) {
    LOG("pixels - set_state - %d", s);
    state = s;
    if (state == pixel::state::OFF) {
        set_pixels(RgbColor(0, 0, 0), false);
        show();
    } else if (get_mode() == pixel::mode::STATIC) {
        set_pixels(to_color(color), true);
    }
}

pixel::color Pixels::get_color() const {
    return color;
}

void Pixels::set_color(pixel::color c) {
    set_color(c, get_mode() == pixel::mode::STATIC);
}

void Pixels::set_color(pixel::color c, const bool refresh) {
    LOG("pixels - set_color - %d %d - %d", c.hue, c.sat, refresh);
    color = c;
    if (refresh) {
        animator.StopAll();
        set_pixels(to_color(color), true);
    }
}

pixel::mode Pixels::get_mode() const {
    for (uint8_t m = pixel::mode::FADE;
         animator.IsAnimating() && m < pixel::mode::TRANSITION;
         m++) {
        if (animator.IsAnimationActive(m)) {
            return (pixel::mode) m;
        }
    }

    return pixel::mode::STATIC;
}

pixel::params Pixels::get_params() const {
    return {
            .duration = this->duration,
            .chained = this->chained,
            .randomized = this->randomized,
    };
}

void Pixels::set_mode(const pixel::mode m, const pixel::params p) {
    LOG("pixels - set_mode - %d %d %d %d", m, p.duration, p.chained, p.randomized);

    this->duration = p.duration;
    this->randomized = p.randomized;
    this->chained = p.chained;

    if (m == pixel::mode::STATIC) {
        LOG("pixels - set_mode - set_color");
        animator.StopAll();
        set_pixels(to_color(color), true);
    } else if (animator.IsAnimationActive(m)) {
        LOG("pixels - set_mode - scale_animation");
        animator.ChangeAnimationDuration(m, 200u + duration);
    } else {
        LOG("pixels - set_mode - set_animation");
        animator.StopAll();
        animator.StartAnimation(m, 200u + duration, animations[m]);
    }
}

void Pixels::set_colors(uint8_t l, pixel::color colors[]) {
    LOG("pixels - set_colors - %d", l);
    animation_colors.clear();
    for (uint8_t i = 0; i < l; i++) {
        LOG("pixels - set_colors - %d %d", colors[i].hue, colors[i].sat);
        animation_colors.push_back(colors[i]);
    }
}

uint8_t Pixels::get_colors_size() const { return this->animation_colors.size(); }

Pixels::Pixels(uint16_t len) : animator(len, NEO_CENTISECONDS),
                               animation_opt({len, 0xffffffffffffu, len, 0}) {
    animations[pixel::mode::FADE] = std::bind(&Pixels::fade, this, std::placeholders::_1);
    animations[pixel::mode::RAINBOW] = std::bind(&Pixels::rainbow, this, std::placeholders::_1);
    animations[pixel::mode::TRANSITION] = std::bind(&Pixels::transition, this, std::placeholders::_1);
}

void Pixels::cycle() {
    if (state == pixel::state::OFF) {
        return;
    }
    if (animator.IsAnimating()) {
        animator.UpdateAnimations();
    }
    show();
}

void Pixels::refresh(const AnimationParam &param) {
    const uint16_t len = length();

    if (param.state == AnimationState_Completed) {
        animator.RestartAnimation(param.index);

        animation_opt.end = randomized ? random(len / 4, len) : len;
        animation_opt.start = randomized ? random(len - animation_opt.end) : 0;
        animation_opt.end = animation_opt.start + animation_opt.end;
        animation_opt.mask = randomized ?
                             (((uint64_t) get_random()) << 32u) + get_random() :
                             0xffffffffffffu;
        LOG("pixels - anim opts - %d~%d %d", animation_opt.start, animation_opt.end, len);
    }
}

static float progress_offset(const uint16_t l, const uint16_t i, float p) {
    p = p + ((float) i / (float) l);
    return p >= 1.0f ? p - 1.0f : p;
}

void Pixels::animate(
        const AnimationParam &param,
        const pixel::ColorSupplier &c,
        const pixel::ColorSupplier &s,
        const bool dim
) {
    refresh(param);

    for (uint16_t i = 0; i < animation_opt.len; i++) {
        if ((animation_opt.mask >> (i % 64)) & 0x1u && !chained) {
            set_pixel(i, s(i, param.progress), true);
        } else if (animation_opt.start <= i && i < animation_opt.end && chained) {
            auto progress = progress_offset(
                    animation_opt.end - animation_opt.start,
                    i - animation_opt.start,
                    param.progress
            );
            set_pixel(i, c(i, progress), true);
        } else if (dim) {
            set_pixel(i, get_pixel(i).Dim(254), false);
        } else {
            set_pixel(i, BLACK, false);
        }
    }
}

static RgbColor linearBlend(const RgbColor &l, const RgbColor &m, const RgbColor &r, float progress) {
    progress = progress * 2;
    if (progress <= 1.0f) {
        return RgbColor::LinearBlend(l, m, progress);
    }
    return RgbColor::LinearBlend(m, r, progress - 1.0f);
}

void Pixels::fade(const AnimationParam &param) {
    const auto rgbColor = RgbColor(to_color(color));

    const pixel::ColorSupplier cs = [&](uint16_t i, float p) {
        return linearBlend(BLACK, rgbColor, BLACK, p);
    };
    this->animate(param, cs, cs, false);
}

void Pixels::rainbow(const AnimationParam &param) {
    const pixel::ColorSupplier cs = [&](uint16_t i, float p) -> HsbColor {
        return {p, 1, 1};
    };

    this->animate(param, cs, cs, true);
}

static HsbColor color_offset(const std::vector<pixel::color> &colors, const float p, const uint16_t i = 0) {
    if (colors.empty()) {
        return BLACK;
    }
    const auto color = colors[
            ((uint16_t) std::floor(colors.size() * p) + i) % colors.size()
    ];
    return to_color(color);
}

void Pixels::transition(const AnimationParam &param) {
    this->animate(param, [&](uint16_t i, float p) {
        return color_offset(animation_colors, p, i);
    }, [&](uint16_t i, float p) {
        return color_offset(animation_colors, p);
    }, false);
}

uint8_t pixel::power_scale(uint16_t mA, uint8_t V, uint16_t len) {
    const double powerApprox = 60.0 * V * len;
    const double powerCap = V * mA;

    if (powerApprox > powerCap) {
        return (uint8_t) (255 * (powerCap / powerApprox));
    } else {
        return 255;
    }
}