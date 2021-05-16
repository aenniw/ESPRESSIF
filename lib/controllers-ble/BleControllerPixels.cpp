#include "BleControllerPixels.h"

void BleControllerPixels::length(BLECharacteristic &c) const {
    auto l = repository.get_length();
    LOG("ble - length %d", l);
    c.setValue(reinterpret_cast<uint8_t *>(&l), 1);
}

bool BleControllerPixels::set_length(BLECharacteristic &c) {
    repository.set_length(c.getData()[0]);
    return true;
}

void BleControllerPixels::power(BLECharacteristic &c) const {
    auto p = repository.get_power();
    LOG("ble - power %d", p);
    c.setValue(p);
}

bool BleControllerPixels::set_power(BLECharacteristic &c) {
    const uint16_t p = c.getData()[0] + (c.getData()[1] << 8);
    repository.set_power(p);
    return true;
}

void BleControllerPixels::state(BLECharacteristic &c) const {
    auto s = (uint8_t) pixels.get_state();
    LOG("ble - state %d", s);
    c.setValue(reinterpret_cast<uint8_t *>(&s), 1);
}

bool BleControllerPixels::set_state(BLECharacteristic &c) {
    const auto s = (pixel::state) c.getData()[0];
    pixels.set_state(s);
    repository.set_state(s);
    return true;
}

void BleControllerPixels::color(BLECharacteristic &c) const {
    auto color = repository.get_color();
    LOG("ble - color %d %d", color.hue, color.sat);
    c.setValue(reinterpret_cast<uint8_t *>(&color), sizeof(pixel::color));
}

bool BleControllerPixels::set_color(BLECharacteristic &c) {
    const pixel::color color = reinterpret_cast<pixel::color *>(c.getData())[0];
    pixels.set_color(color);
    repository.set_color(color);
    return true;
}

void BleControllerPixels::brightness(BLECharacteristic &c) const {
    auto b = pixels.get_brightness();
    LOG("ble - brightness %d", b);
    c.setValue(reinterpret_cast<uint8_t *>(&b), 1);
}

bool BleControllerPixels::set_brightness(BLECharacteristic &c) {
    auto b = c.getData()[0];
    pixels.set_brightness(b);
    repository.set_brightness(b);
    return true;
}

void BleControllerPixels::mode(BLECharacteristic &c) const {
    auto m = (uint8_t) repository.get_mode();
    auto p = repository.get_params();
    const auto len = 1 + sizeof(pixel::params);

    LOG("ble - mode %d %d %d %d", m, p.duration, p.randomized, p.chained);
    uint8_t data[len] = {m};
    memcpy(data + 1, reinterpret_cast<uint8_t *>(&p), sizeof(pixel::params));
    c.setValue(data, len);
}

bool BleControllerPixels::set_mode(BLECharacteristic &c) {
    const auto m = (pixel::mode) c.getData()[0];
    const pixel::params p = reinterpret_cast<pixel::params *>(c.getData() + 1)[0];

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
    auto len = c.getValue().size() / sizeof(pixel::color);
    auto data = reinterpret_cast<pixel::color *>(c.getData());
    pixels.set_colors(len, data);
    repository.set_colors(len, data);
    return true;
}

void BleControllerPixels::subscribe(BleServer &ble) {
    repository.configure(pixels);

    ble.on(UUID, UUID_POWER,
           std::bind(&BleControllerPixels::power, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_power, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED, 30);
    ble.on(UUID, UUID_LENGTH,
           std::bind(&BleControllerPixels::length, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_length, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    stateCharacteristic = ble.on(
            UUID, UUID_STATE,
            std::bind(&BleControllerPixels::state, this, std::placeholders::_1),
            std::bind(&BleControllerPixels::set_state, this, std::placeholders::_1),
            ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED
    );
    ble.on(UUID, UUID_COLOR,
           std::bind(&BleControllerPixels::color, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_color, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    brightnessCharacteristic = ble.on(
            UUID, UUID_BRIGHTNESS,
            std::bind(&BleControllerPixels::brightness, this, std::placeholders::_1),
            std::bind(&BleControllerPixels::set_brightness, this, std::placeholders::_1),
            ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED
    );
    modeCharacteristic = ble.on(
            UUID, UUID_MODE,
            std::bind(&BleControllerPixels::mode, this, std::placeholders::_1),
            std::bind(&BleControllerPixels::set_mode, this, std::placeholders::_1),
            ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED
    );
    ble.on(UUID, UUID_COLORS,
           std::bind(&BleControllerPixels::colors, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_colors, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
}

void BleControllerPixels::toggle() {
    repository.set_state(
            repository.get_state() == pixel::ON ? pixel::OFF : pixel::ON
    );

    state(*stateCharacteristic);
    set_state(*stateCharacteristic);
    stateCharacteristic->notify();
}

void BleControllerPixels::brighten() {
    auto value = repository.get_brightness();
    repository.set_brightness(
            value == 255u ? 0u : max(value + 5u, 255u)
    );

    brightness(*brightnessCharacteristic);
    set_brightness(*brightnessCharacteristic);
    brightnessCharacteristic->notify();
}

void BleControllerPixels::speedup() {
    pixel::params value = repository.get_params();
    value.duration = value.duration == 1800u ?
                     0u : max(value.duration + 50u, 1800u);

    repository.set_params(value);
    mode(*modeCharacteristic);
    set_mode(*modeCharacteristic);
    modeCharacteristic->notify();
}

void BleControllerPixels::toggle_modes() {
    pixel::mode m = repository.get_mode();
    pixel::params p = repository.get_params();
    if (p.randomized && p.chained) {
        m = (pixel::mode) ((m + 1) % (pixel::TRANSITION + 1));
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
