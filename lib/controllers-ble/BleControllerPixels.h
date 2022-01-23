#pragma once

#include <commons.h>
#include <Pixels.h>
#include <BleServer.h>
#include <PixelsRepository.h>

class BleControllerPixels : public Subscriber<BleServer> {
public:
    const static NimBLEUUID UUID, UUID_COLOR, UUID_BRIGHTNESS, UUID_MODE,
            UUID_COLORS, UUID_STATE, UUID_POWER, UUID_LENGTH;
private:
    Pixels &pixels;
    PixelsRepository &repository;

    BLECharacteristic *stateCharacteristic = nullptr;
    BLECharacteristic *modeCharacteristic = nullptr;
    BLECharacteristic *brightnessCharacteristic = nullptr;
protected:
    void length(BLECharacteristic &c) const;
    bool set_length(BLECharacteristic &c);
    void power(BLECharacteristic &c) const;
    bool set_power(BLECharacteristic &c);
    void state(BLECharacteristic &c) const;
    bool set_state(BLECharacteristic &c);
    void color(BLECharacteristic &c) const;
    bool set_color(BLECharacteristic &c);
    void brightness(BLECharacteristic &c) const;
    bool set_brightness(BLECharacteristic &c);
    void mode(BLECharacteristic &c) const;
    bool set_mode(BLECharacteristic &c);
    void colors(BLECharacteristic &c) const;
    bool set_colors(BLECharacteristic &c);
public:
    explicit BleControllerPixels(Pixels &pixels, PixelsRepository &repository) :
            pixels(pixels), repository(repository) {}

    void toggle();
    void brighten();
    void speedup();
    void toggle_modes();

    void subscribe(BleServer &ble) override;
};
