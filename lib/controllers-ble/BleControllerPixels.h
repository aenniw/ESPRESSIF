#pragma once

#include <commons.h>
#include <Pixels.h>
#include <BleServer.h>
#include <PixelsRepository.h>

class BleControllerPixels : public Subscriber<BleServer> {
public:
    const static NimBLEUUID UUID_COLOR, UUID_BRIGHTNESS, UUID_MODE,
            UUID_COLORS, UUID_STATE, UUID_LENGTH;
private:
    Pixels &pixels;
    PixelsRepository &repository;
    const NimBLEUUID UUID;

    BLECharacteristic *colorCharacteristic = nullptr;
    BLECharacteristic *stateCharacteristic = nullptr;
    BLECharacteristic *modeCharacteristic = nullptr;
    BLECharacteristic *brightnessCharacteristic = nullptr;
protected:
    void length(BLECharacteristic &c) const;
    bool set_length(BLECharacteristic &c);
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
    explicit BleControllerPixels(Pixels &pixels, PixelsRepository &repository,
                                 uint8_t i = 0, uint32_t uuid = 0xab5ff770) :
            pixels(pixels), repository(repository), UUID(fullUUID(uuid + i)) {}

    void toggle_state();
    void toggle_modes();
    void cycle_brightness(bool notify);
    void cycle_color(bool notify);
    void cycle_speed(bool notify);

    void subscribe(BleServer &ble) override;
};
