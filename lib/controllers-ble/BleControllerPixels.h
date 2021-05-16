#pragma once

#include <commons.h>
#include <Pixels.h>
#include <BleServer.h>
#include <PixelsRepository.h>

class BleControllerPixels : public Subscriber<BleServer>, public Service {
private:
    const uint32_t UUID = 0xab5ff770;
    Pixels &pixels;
    PixelsRepository repository;
protected:
    void color(BLECharacteristic &c) const;
    void set_color(BLECharacteristic &c);
    void brightness(BLECharacteristic &c) const;
    void set_brightness(BLECharacteristic &c);
    void animation(BLECharacteristic &c) const;
    void set_animation(BLECharacteristic &c);
    void colors(BLECharacteristic &c) const;
    void set_colors(BLECharacteristic &c);
public:
    explicit BleControllerPixels(FileSystem &fs, Pixels &pixels) :
            pixels(pixels), repository(fs) {}

    void begin() override;
    void subscribe(BleServer &ble) override;
    void cycle() override {};
};
