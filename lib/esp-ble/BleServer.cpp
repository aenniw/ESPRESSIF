#include "BleServer.h"

void BleServer::rm_bonds() {
    auto dev_num = esp_ble_get_bond_device_num();
    esp_ble_bond_dev_t dev_list[dev_num];
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    LOG("ble - reset bonds %d", dev_num);
    for (int i = 0; i < dev_num; i++) {
        esp_ble_remove_bond_device(dev_list[i].bd_addr);
    }
}

BLEService *BleServer::service(const uint32_t sUuid, uint32_t numHandles) {
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
        server = BLEDevice::createServer();
    } else { return; }

    for (const auto &subscription : subscriptions)
        subscription->subscribe(*this);

    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    security.setStaticPIN(secret);
    security.setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);

    for (auto &controller : controllers)
        server->getServiceByUUID(controller)->start();

    auto advertising = BLEDevice::getAdvertising();
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);
    advertData.setName(name);
    advertData.setManufacturerData(manufacturer);
    advertising->setAdvertisementData(advertData);
    advertising->start();
}

BLECharacteristic *BleServer::on(const uint32_t sUuid, const uint32_t uuid, const BleRHandler &handler,
                   const esp_gatt_perm_t perm, const uint32_t numHandles) {
    auto pCharacteristic = service(sUuid, numHandles)->createCharacteristic(
            uuid, BLECharacteristic::PROPERTY_READ
    );
    pCharacteristic->setCallbacks(new BleCallback(handler));
    pCharacteristic->setAccessPermissions(perm);
    return pCharacteristic;
}

BLECharacteristic *BleServer::on(const uint32_t sUuid, const uint32_t uuid, const BleWHandler &handler,
                   const esp_gatt_perm_t perm, const uint32_t numHandles) {
    auto pCharacteristic = service(sUuid, numHandles)->createCharacteristic(
            uuid, BLECharacteristic::PROPERTY_WRITE |
                  BLECharacteristic::PROPERTY_INDICATE
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new BleCallback(handler));
    pCharacteristic->setAccessPermissions(perm);
    return pCharacteristic;
}

BLECharacteristic *BleServer::on(const uint32_t sUuid, const uint32_t uuid, const BleRHandler &read, const BleWHandler &write,
                   const esp_gatt_perm_t perm, const uint32_t numHandles) {
    auto pCharacteristic = service(sUuid, numHandles)->createCharacteristic(
            uuid, BLECharacteristic::PROPERTY_READ |
                  BLECharacteristic::PROPERTY_WRITE |
                  BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new BleCallback(read, write));
    pCharacteristic->setAccessPermissions(perm);
    return pCharacteristic;
}

void BleServer::serve(Subscriber<BleServer> *c) {
    this->subscriptions.push_back(c);
}
