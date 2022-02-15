#include "Haptic.h"
#include <elapsedMillis.h>

void Haptic::begin() {
    leftButton.setClickTicks(300);
    leftButton.setPressTicks(400);
    leftButton.attachClick([](void *param) { ((Haptic *) param)->cycle_chains(); }, this);
    leftButton.attachDoubleClick([](void *param) { ((Haptic *) param)->toggle_state(); }, this);
    leftButton.attachDuringLongPress([](void *param) { ((Haptic *) param)->cycle_brightness(); }, this);
    leftButton.attachLongPressStop([](void *param) { ((Haptic *) param)->save_brightness(); }, this);

    rightButton.setClickTicks(300);
    rightButton.setPressTicks(400);
    rightButton.attachClick([](void *param) { ((Haptic *) param)->toggle_modes(); }, this);
    rightButton.attachDuringLongPress([](void *param) { ((Haptic *) param)->cycle_options(); }, this);
    rightButton.attachLongPressStop([](void *param) { ((Haptic *) param)->save_options(); }, this);

    for (size_t i = 0; i < indicators.size(); i++) {
        pinMode(indicators[i], OUTPUT);
        digitalWrite(indicators[i], i == selected ? HIGH : LOW);
    }
}

void Haptic::cycle() {
    leftButton.tick();
    rightButton.tick();
}

void Haptic::toggle_modes() {
    LOG("haptics - modes");
    controllers[selected]->toggle_modes();
}

void Haptic::cycle_options() {
    static elapsedMillis elapsed;

    if (elapsed < 50) return;
    LOG("haptics - speed");
    controllers[selected]->cycle_speed(false);
    controllers[selected]->cycle_color(false);
    elapsed = 0;
}

void Haptic::save_options() {
    LOG("haptics - save speed");
    controllers[selected]->cycle_speed(true);
    controllers[selected]->cycle_color(true);
}

void Haptic::cycle_brightness() {
    static elapsedMillis elapsed;

    if (elapsed < 50) return;
    LOG("haptics - brightness");
    controllers[selected]->cycle_brightness(false);
    elapsed = 0;
}

void Haptic::save_brightness() {
    LOG("haptics - save brightness");
    controllers[selected]->cycle_brightness(true);
}

void Haptic::toggle_state() {
    LOG("haptics - state");
    controllers[selected]->toggle_state();
}

void Haptic::cycle_chains() {
    selected = (selected + 1) % controllers.size();
    LOG("haptics - strand %d", selected);

    for (size_t i = 0; i < indicators.size(); i++) {
        digitalWrite(indicators[i], i == selected ? HIGH : LOW);
    }
}

void Haptic::serve(BleControllerPixels *c) {
    controllers.push_back(c);
}
