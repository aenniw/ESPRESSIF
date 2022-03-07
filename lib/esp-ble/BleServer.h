#pragma once

#include <commons.h>
#include <functional>
#include <NimBLEDevice.h>

typedef std::function<void(BLECharacteristic &c)> BleRHandler;
typedef std::function<bool(BLECharacteristic &c)> BleWHandler;

class BleCallback : public BLECharacteristicCallbacks {
private:
    const BleRHandler read;
    const BleWHandler write;
public:
    explicit BleCallback(BleRHandler read);
    explicit BleCallback(BleWHandler write);
    BleCallback(BleRHandler read, BleWHandler write);

    void onRead(BLECharacteristic *pCharacteristic) override;
    void onWrite(BLECharacteristic *pCharacteristic) override;

};

class BleServer : public Service, public BLEServerCallbacks {
private:
    std::string name, manufacturer;
    uint32_t secret;
    BLEAdvertisementData advertData;
    BLEServer *server = nullptr;
    std::vector<NimBLEUUID> controllers;
    std::vector<Subscriber<BleServer> *> subscriptions;
protected:
    BLEService *service(NimBLEUUID sUuid, uint32_t numHandles = 15);
public:
    explicit BleServer(std::string name, std::string manufacturer, uint32_t secret);

    void begin() override;
    void cycle() override {}

    BLECharacteristic *on(NimBLEUUID sUuid, NimBLEUUID uuid, const BleRHandler &handler, uint32_t numHandles = 15);
    BLECharacteristic *on(NimBLEUUID sUuid, NimBLEUUID uuid, const BleWHandler &handler, uint32_t numHandles = 15);
    BLECharacteristic *on(NimBLEUUID sUuid, NimBLEUUID uuid, const BleRHandler &read, const BleWHandler &write,
                          uint32_t numHandles = 15);

    void serve(Subscriber<BleServer> *c);
    void onConnect(BLEServer *pServer) override;

    static void rm_bonds();
};

NimBLEUUID fullUUID(uint32_t uuid);