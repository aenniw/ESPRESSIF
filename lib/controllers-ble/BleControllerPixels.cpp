#include "BleControllerPixels.h"

void BleControllerPixels::length(BLECharacteristic &c) const {
    auto l = repository.get_length();
    LOG("ble - length %d", l);
    c.setValue(l);
}

bool BleControllerPixels::set_length(BLECharacteristic &c) {
    repository.set_length(c.getValue<uint8_t>());
    return true;
}

void BleControllerPixels::state(BLECharacteristic &c) const {
    auto s = (uint8_t) repository.get_state();
    LOG("ble - state %d", s);
    c.setValue(s);
}

bool BleControllerPixels::set_state(BLECharacteristic &c) {
    const auto s = (pixel::state) c.getValue<uint8_t>();
    pixels.set_state(s);
    repository.set_state(s);
    return true;
}

void BleControllerPixels::color(BLECharacteristic &c) const {
    auto color = repository.get_color();
    LOG("ble - color %d %d", color.hue, color.sat);
    c.setValue(color);
}

bool BleControllerPixels::set_color(BLECharacteristic &c) {
    const pixel::color color = c.getValue<pixel::color>();
    pixels.set_color(color);
    repository.set_color(color);
    return true;
}

void BleControllerPixels::brightness(BLECharacteristic &c) const {
    auto b = repository.get_brightness();
    LOG("ble - brightness %d", b);
    c.setValue(b);
}

bool BleControllerPixels::set_brightness(BLECharacteristic &c) {
    auto b = c.getValue<uint8_t>();
    pixels.set_brightness(b);
    repository.set_brightness(b);
    return true;
}

typedef struct {
    uint32_t mode: 8;
    uint32_t duration: 16;
    uint32_t chained: 1;
    uint32_t randomized: 1;
    uint32_t : 6;
} mode_msg;

void BleControllerPixels::mode(BLECharacteristic &c) const {
    auto p = repository.get_params();
    mode_msg m = {
            .mode = (uint8_t) repository.get_mode(),
            .duration = p.duration,
            .chained = p.chained,
            .randomized = p.randomized
    };

    LOG("ble - mode %d %d %d %d", m.mode, m.duration, m.randomized, m.chained);
    c.setValue(m);
}

bool BleControllerPixels::set_mode(BLECharacteristic &c) {
    const auto data = c.getValue<mode_msg>();
    const auto m = (pixel::mode) data.mode;
    const pixel::params p = {
            .duration = data.duration,
            .chained = data.chained,
            .randomized = data.randomized,
    };

    pixels.set_mode(m, p);
    repository.set_mode(m);
    repository.set_params(p);
    return true;
}

void BleControllerPixels::colors(BLECharacteristic &c) const {
    repository.get_colors([&](uint8_t l, pixel::color colors[]) {
        LOG("ble - colors %d", l);
        c.setValue(reinterpret_cast<uint8_t *>(colors), l * sizeof(pixel::color));
    });
}

bool BleControllerPixels::set_colors(BLECharacteristic &c) {
    auto msg = c.getValue();
    auto len = msg.length() / sizeof(pixel::color);
    auto colors = reinterpret_cast<pixel::color *>((uint8_t *) msg.data());
    pixels.set_colors(len, colors);
    repository.set_colors(len, colors);
    return true;
}

const NimBLEUUID  BleControllerPixels::UUID_COLOR = fullUUID(0x05f3704eu);
const NimBLEUUID  BleControllerPixels::UUID_BRIGHTNESS = fullUUID(0x604d979du);
const NimBLEUUID  BleControllerPixels::UUID_MODE = fullUUID(0xa7601c29u);
const NimBLEUUID  BleControllerPixels::UUID_COLORS = fullUUID(0xb532fb4eu);
const NimBLEUUID  BleControllerPixels::UUID_STATE = fullUUID(0xb533fb4eu);
const NimBLEUUID  BleControllerPixels::UUID_LENGTH = fullUUID(0xb535fb4eu);

void BleControllerPixels::subscribe(BleServer &ble) {
    repository.configure(pixels);

    ble.on(UUID, UUID_LENGTH,
           std::bind(&BleControllerPixels::length, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_length, this, std::placeholders::_1), 26);
    stateCharacteristic = ble.on(
            UUID, UUID_STATE,
            std::bind(&BleControllerPixels::state, this, std::placeholders::_1),
            std::bind(&BleControllerPixels::set_state, this, std::placeholders::_1)
    );
    colorCharacteristic = ble.on(
            UUID, UUID_COLOR,
            std::bind(&BleControllerPixels::color, this, std::placeholders::_1),
            std::bind(&BleControllerPixels::set_color, this, std::placeholders::_1));
    brightnessCharacteristic = ble.on(
            UUID, UUID_BRIGHTNESS,
            std::bind(&BleControllerPixels::brightness, this, std::placeholders::_1),
            std::bind(&BleControllerPixels::set_brightness, this, std::placeholders::_1)
    );
    modeCharacteristic = ble.on(
            UUID, UUID_MODE,
            std::bind(&BleControllerPixels::mode, this, std::placeholders::_1),
            std::bind(&BleControllerPixels::set_mode, this, std::placeholders::_1)
    );
    ble.on(UUID, UUID_COLORS,
           std::bind(&BleControllerPixels::colors, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_colors, this, std::placeholders::_1));
}

void BleControllerPixels::toggle_state() {
    repository.set_state(
            repository.get_state() == pixel::ON ? pixel::OFF : pixel::ON
    );

    state(*stateCharacteristic);
    set_state(*stateCharacteristic);
    stateCharacteristic->notify();
}

void BleControllerPixels::cycle_brightness(bool notify) {
    if (repository.get_state() == pixel::OFF) {
        return;
    }

    static int inc = 5;
    const auto value = pixels.get_brightness();
    if (notify) {
        repository.set_brightness(value);
        brightness(*brightnessCharacteristic);
        set_brightness(*brightnessCharacteristic);
        brightnessCharacteristic->notify();
    } else {
        inc = value == 255 || value <= 10 ? -inc : inc;
        pixels.set_brightness(min(max(value + inc, 10), 255));
    }
}

void BleControllerPixels::cycle_color(bool notify) {
    if (repository.get_state() == pixel::OFF ||
        repository.get_mode() != pixel::STATIC) {
        return;
    }

    const uint16_t inc = 10u;
    auto value = pixels.get_color();
    if (notify) {
        repository.set_color(value);
        color(*colorCharacteristic);
        set_color(*colorCharacteristic);
        colorCharacteristic->notify();
    } else {
        if (value.hue + inc > 360u) {
            value.sat = (value.sat + inc) % 101u;
        }
        value.hue = (value.hue + inc) % 361u;
        pixels.set_color(value);
    }
}

void BleControllerPixels::cycle_speed(bool notify) {
    if (repository.get_state() == pixel::OFF ||
        repository.get_mode() == pixel::STATIC) {
        return;
    }

    static int inc = 25;
    auto value = pixels.get_params();
    if (notify) {
        repository.set_params(value);
        mode(*modeCharacteristic);
        set_mode(*modeCharacteristic);
        modeCharacteristic->notify();
    } else {
        inc = value.duration % 1800u == 0 ? -inc : inc;
        value.duration = min(max((int) value.duration + inc, 0), 1800);
        pixels.set_mode(pixels.get_mode(), value);
    }
}

void BleControllerPixels::toggle_modes() {
    if (repository.get_state() == pixel::OFF) {
        return;
    }

    pixel::mode m = repository.get_mode();
    pixel::params p = repository.get_params();

    if (m == pixel::STATIC) {
        m = (pixel::mode) ((m + 1) % (pixel::TRANSITION + 1));
    } else if (p.randomized && p.chained) {
        do {
            m = (pixel::mode) ((m + 1) % (pixel::TRANSITION + 1));
        } while (m == pixel::TRANSITION && pixels.get_colors_size() == 0);

        p.randomized = false;
        p.chained = false;
    } else if (!p.chained) {
        p.chained = true;
    } else if (!p.randomized) {
        p.randomized = true;
    }

    repository.set_mode(m);
    repository.set_params(p);
    mode(*modeCharacteristic);
    set_mode(*modeCharacteristic);
    modeCharacteristic->notify();
}
