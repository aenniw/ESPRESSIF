#pragma once

#include <OneButton.h>
#include <BleControllerPixels.h>

class Haptic : public Service {
private:
    OneButton leftButton, rightButton;
    std::vector<BleControllerPixels *> controllers;
    std::vector<uint8_t> indicators;
    uint8_t selected = 0u;
protected:
    void cycle_chains();
    void toggle_state();
    void toggle_modes();
    void cycle_brightness();
    void save_brightness();
    void cycle_options();
    void save_options();
public:
    Haptic(uint8_t left, uint8_t right, std::vector<uint8_t> indicators = {}) :
            leftButton(left, true, true),
            rightButton(right, true, true),
            indicators(std::move(indicators)) {};

    void serve(BleControllerPixels *c);

    void begin() override;;
    void cycle() override;
};