#pragma once

#include <commons.h>
#include <functional>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>

typedef std::function<void(BLECharacteristic &c)> BleRHandler;
typedef std::function<bool(BLECharacteristic &c)> BleWHandler;

class BleCallback : public BLECharacteristicCallbacks {
private:
    const BleRHandler read;
    const BleWHandler write;
public:
    explicit BleCallback(BleRHandler read) : read(std::move(read)), write(nullptr) {}
    explicit BleCallback(BleWHandler write) : read(nullptr), write(std::move(write)) {}
    BleCallback(BleRHandler read, BleWHandler write) : read(std::move(read)), write(std::move(write)) {}

    void onRead(BLECharacteristic *pCharacteristic) override {
        if (read && pCharacteristic->getValue().empty()) {
            read(*pCharacteristic);
        }
    };

    void onWrite(BLECharacteristic *pCharacteristic) override {
        if (write && write(*pCharacteristic)) {
            pCharacteristic->notify();
        }
    }

};

class BleServer : public Service {
private:
    std::string name, manufacturer;
    uint32_t secret;
    BLEAdvertisementData advertData;
    BLESecurity security;
    BLEServer *server = nullptr;
    std::vector<uint32_t> controllers;
    std::vector<Subscriber<BleServer> *> subscriptions;
protected:
    BLEService *service(uint32_t sUuid, uint32_t numHandles = 15);
public:
    explicit BleServer(std::string name, std::string manufacturer, uint32_t secret);

    void begin() override;

    void cycle() override {}

    static void rm_bonds();

    BLECharacteristic *on(uint32_t sUuid, uint32_t uuid, const BleRHandler &handler,
                          esp_gatt_perm_t perm = ESP_GATT_PERM_READ, uint32_t numHandles = 15);

    BLECharacteristic *on(uint32_t sUuid, uint32_t uuid, const BleWHandler &handler,
                          esp_gatt_perm_t perm = ESP_GATT_PERM_WRITE, uint32_t numHandles = 15);

    BLECharacteristic *on(uint32_t sUuid, uint32_t uuid, const BleRHandler &read, const BleWHandler &write,
                          esp_gatt_perm_t perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                          uint32_t numHandles = 15);

    void serve(Subscriber<BleServer> *c);
};