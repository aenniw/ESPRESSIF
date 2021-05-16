#pragma once

#include <commons.h>
#include <functional>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>

typedef std::function<void(BLECharacteristic &c)> BleHandler;

class BleR : public BLECharacteristicCallbacks {
private:
    const BleHandler read;
public:
    explicit BleR(BleHandler read);
    void onRead(BLECharacteristic *pCharacteristic) override;
};

class BleRW : public BleR {
private:
    const BleHandler write;
public:
    BleRW(const BleHandler &read, BleHandler write);
    void onWrite(BLECharacteristic *pCharacteristic) override;
};


class BleServer : public Service, public BLEServerCallbacks {
private:
    const char *name;
    uint32_t pin;
    BLE2902 ble2902;
    BLESecurity security;
    BLEServer *server = nullptr;
    std::vector<uint32_t> controllers;
protected:
    void initialize();
    BLEService *service(uint32_t sUuid);

public:
    explicit BleServer(const char *name, uint32_t pin);

    void begin() override;
    void cycle() override {}

    void on(uint32_t sUuid, uint32_t uuid, const BleHandler &read,
            esp_gatt_perm_t perm = ESP_GATT_PERM_READ);
    void on(uint32_t sUuid, uint32_t uuid, const BleHandler &read, const BleHandler &write,
            esp_gatt_perm_t perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
    void onConnect(BLEServer *pServer) override;
    BleServer &serve(Subscriber<BleServer> &c);

};