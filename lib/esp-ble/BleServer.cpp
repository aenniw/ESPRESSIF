#include "BleServer.h"

BleR::BleR(BleHandler read) : read(std::move(read)) {}

void BleR::onRead(BLECharacteristic *pCharacteristic) {
    read(*pCharacteristic);
}

BleRW::BleRW(const BleHandler &read, BleHandler write) : BleR(read), write(std::move(write)) {}

void BleRW::onWrite(BLECharacteristic *pCharacteristic) {
    write(*pCharacteristic);
}

void BleServer::initialize() {
    if (server == nullptr) {
        BLEDevice::init(name);
        server = BLEDevice::createServer();
    }
}

BLEService *BleServer::service(const uint32_t sUuid) {
    initialize();
    auto service = server->getServiceByUUID(sUuid);
    if (service == nullptr) {
        service = server->createService(sUuid);
        controllers.push_back(sUuid);
    }
    return service;
}

BleServer::BleServer(const char *name, const uint32_t pin) : name(name), pin(pin) {}

void BleServer::begin() {
    initialize();

    if (pin) {
        BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
        security.setStaticPIN(pin);
        security.setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    }

    server->setCallbacks(this);
    for (auto &controller : controllers)
        server->getServiceByUUID(controller)->start();

    auto advertising = BLEDevice::getAdvertising();
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);
    advertising->start();
}

void BleServer::on(const uint32_t sUuid, const uint32_t uuid, const BleHandler &read, const esp_gatt_perm_t perm) {
    auto pCharacteristic = service(sUuid)->createCharacteristic(
            uuid, BLECharacteristic::PROPERTY_READ
    );
    pCharacteristic->setCallbacks(new BleR(read));
    pCharacteristic->addDescriptor(&ble2902);
    pCharacteristic->setAccessPermissions(perm);
}

void BleServer::on(const uint32_t sUuid, const uint32_t uuid, const BleHandler &read, const BleHandler &write,
                   const esp_gatt_perm_t perm) {
    auto pCharacteristic = service(sUuid)->createCharacteristic(
            uuid, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new BleRW(read, write));
    pCharacteristic->addDescriptor(&ble2902);
    pCharacteristic->setAccessPermissions(perm);
}

void BleServer::onConnect(BLEServer *pServer) {
    BLEDevice::startAdvertising();
}

BleServer &BleServer::serve(Subscriber<BleServer> &c) {
    c.subscribe(*this);
    return *this;
}
