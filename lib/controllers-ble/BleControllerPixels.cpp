#include "BleControllerPixels.h"

using animation::type;

typedef enum {
    RM, ADD
} operation;

void BleControllerPixels::begin() {
    pixels.set_color(HtmlColor(repository.get_color()), false);
    repository.get_colors([&](const uint32_t color) {
        pixels.add_color(HtmlColor(color));
    });
    pixels.set_animation(
            repository.get_animation_type(),
            repository.get_animation_duration()
    );

    if (pixels.animation() == type::NONE) {
        pixels.set_color(pixels.get_color());
    }
}

void BleControllerPixels::color(BLECharacteristic &c) const {
    char color[9] = "0x000000";
    snprintf(color, 9, "0x%06X", repository.get_color());
    c.setValue(color);
}

void BleControllerPixels::set_color(BLECharacteristic &c) {
    auto color = HtmlColor();
    if (!color.Parse<HtmlShortColorNames>(c.getValue().c_str())) {
        return;
    }

    pixels.set_color(color);
    repository.set_color(color);
    repository.set_animation_type(type::NONE);
}

void BleControllerPixels::brightness(BLECharacteristic &c) const {
    auto b = pixels.get_brightness();
    c.setValue(b);
}

void BleControllerPixels::set_brightness(BLECharacteristic &c) {
    const auto b = String(c.getValue().c_str()).toFloat();
    if (!pixels.set_brightness(b)) {
        return;
    }
    repository.set_color(HtmlColor(RgbColor(pixels.get_color())));
}

void BleControllerPixels::animation(BLECharacteristic &c) const {
    auto animation = repository.get_animation_type();
    auto duration = repository.get_animation_duration();

    uint8_t data[3] = {
            (uint8_t) animation,
            (uint8_t) (duration >> 8u),
            (uint8_t) (duration & 0xffu)
    };
    c.setValue(data, 3);
}

void BleControllerPixels::set_animation(BLECharacteristic &c) {
    const type t = (type) c.getData()[0];
    const uint16_t d = (c.getData()[1] << 8u) + c.getData()[2];
    if (!pixels.set_animation(t, d)) {
        return;
    }

    repository.set_animation_type(t);
    repository.set_animation_duration(d);
}

void BleControllerPixels::colors(BLECharacteristic &c) const {
    uint8_t len = 20, data[len];

    repository.get_colors([&](uint32_t color) {
        uint8_t i = data[0] * 3;
        data[i + 1] = (color & 0xff0000u) >> 16u;
        data[i + 2] = (color & 0xff00u) >> 8u;
        data[i + 3] = color & 0xffu;
        data[0]++;
    });

    c.setValue(data, len);
}

void BleControllerPixels::set_colors(BLECharacteristic &c) {
    const auto op = (operation) c.getData()[0];
    if (op == RM) {
        auto pixel = c.getData()[1];
        if (pixel < 0 || !pixels.rm_color(pixel)) {
            return;
        }

        repository.rm_color(pixel);
    } else if (op == ADD) {
        auto color = HtmlColor();
        if (!color.Parse<HtmlShortColorNames>((char *) (c.getData() + 1))) {
            return;
        }

        repository.add_color(color);
    }
}

void BleControllerPixels::subscribe(BleServer &ble) {
    ble.on(UUID, 0x05f3704e,
           std::bind(&BleControllerPixels::color, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_color, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    ble.on(UUID, 0x604d979d,
           std::bind(&BleControllerPixels::brightness, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_brightness, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    ble.on(UUID, 0xa7601c29,
           std::bind(&BleControllerPixels::animation, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_animation, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    ble.on(UUID, 0xb532fb4e,
           std::bind(&BleControllerPixels::colors, this, std::placeholders::_1),
           std::bind(&BleControllerPixels::set_colors, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
}
