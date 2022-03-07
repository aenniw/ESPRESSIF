#include "BleServer.h"
#include <esp_gap_ble_api.h>

void BleServer::rm_bonds() {
    LOG("ble - reset bonds");
    NimBLEDevice::deleteAllBonds();
}

BLEService *BleServer::service(const NimBLEUUID sUuid, uint32_t numHandles) {
    auto service = server->getServiceByUUID(sUuid);
    if (service == nullptr) {
        service = server->createService(sUuid, numHandles);
        controllers.push_back(sUuid);
    }
    return service;
}

BleServer::BleServer(std::string name, std::string manufacturer, const uint32_t secret) :
        name(std::move(name)), manufacturer(std::move(manufacturer)), secret(secret) {}

void BleServer::begin() {
    if (server == nullptr) {
        BLEDevice::init(name);
        BLEDevice::setPower(ESP_PWR_LVL_P9);

        BLEDevice::setSecurityAuth(true, true, true);
        BLEDevice::setSecurityPasskey(secret);
        BLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

        server = BLEDevice::createServer();
    } else { return; }

    for (const auto &subscription : subscriptions)
        subscription->subscribe(*this);

    server->setCallbacks(this);
    for (auto &controller : controllers)
        server->getServiceByUUID(controller)->start();

    advertData.setName(name);
    advertData.setManufacturerData(manufacturer);

    auto advertising = BLEDevice::getAdvertising();
    advertising->setScanResponse(true);
    advertising->setAdvertisementData(advertData);
    advertising->start();
}

BLECharacteristic *BleServer::on(const NimBLEUUID sUuid, const NimBLEUUID uuid, const BleRHandler &handler,
                                 const uint32_t numHandles) {
    auto pCharacteristic = service(sUuid, numHandles)->createCharacteristic(
            uuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC
    );
    pCharacteristic->setCallbacks(new BleCallback(handler));
    return pCharacteristic;
}

BLECharacteristic *BleServer::on(const NimBLEUUID sUuid, const NimBLEUUID uuid, const BleWHandler &handler,
                                 const uint32_t numHandles) {
    auto pCharacteristic = service(sUuid, numHandles)->createCharacteristic(
            uuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC |
                  NIMBLE_PROPERTY::INDICATE
    );
    pCharacteristic->setCallbacks(new BleCallback(handler));
    return pCharacteristic;
}

BLECharacteristic *
BleServer::on(const NimBLEUUID sUuid, const NimBLEUUID uuid, const BleRHandler &read, const BleWHandler &write,
              const uint32_t numHandles) {
    auto pCharacteristic = service(sUuid, numHandles)->createCharacteristic(
            uuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC |
                  NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC |
                  NIMBLE_PROPERTY::NOTIFY
    );
    pCharacteristic->setCallbacks(new BleCallback(read, write));
    return pCharacteristic;
}

void BleServer::serve(Subscriber<BleServer> *c) {
    this->subscriptions.push_back(c);
}

void BleServer::onConnect(BLEServer *s) {
    BLEDevice::startAdvertising();
}

BleCallback::BleCallback(BleRHandler read) : read(std::move(read)), write(nullptr) {}

BleCallback::BleCallback(BleWHandler write) : read(nullptr), write(std::move(write)) {}

BleCallback::BleCallback(BleRHandler read, BleWHandler write) : read(std::move(read)), write(std::move(write)) {}

void BleCallback::onRead(BLECharacteristic *pCharacteristic) {
    if (read && pCharacteristic->getDataLength() == 0) {
        read(*pCharacteristic);
    }
}

void BleCallback::onWrite(BLECharacteristic *pCharacteristic) {
    if (write && write(*pCharacteristic)) {
        pCharacteristic->notify();
    }
}

NimBLEUUID fullUUID(uint32_t uuid) {
    return {uuid, 0x0000, 0x1000u, (uint64_t(0x8000u) << 48u) + 0x00805f9b34fbu};
}
