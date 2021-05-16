#include <BleControllerUtil.h>

void BleControllerUtil::hardware(BLECharacteristic &c) const {
    LOG("ble - hw %s", hw_version.c_str());
    c.setValue(hw_version);
}

void BleControllerUtil::firmware(BLECharacteristic &c) const {
    LOG("ble - fw %s", fw_version.c_str());
    c.setValue(fw_version);
}

bool BleControllerUtil::set_name(BLECharacteristic &c) {
    const auto name = c.getValue();
    if (name.length() <= 30) {
        repository.set_name(name);
    }
    return false;
}

bool BleControllerUtil::set_secret(BLECharacteristic &c) {
    const auto pin = c.getValue();
    if (pin.length() >= 6 && pin.length() <= 9) {
        repository.set_secret(
                (uint32_t) strtol(pin.c_str(), nullptr, 10)
        );
        BleServer::rm_bonds();
    }
    return false;
}

bool BleControllerUtil::set_firmware(BLECharacteristic &c) {
    std::string data = c.getValue();
    if (!updateFlag) {
        LOG("ble - ota - start");
        esp_ota_begin(esp_ota_get_next_update_partition(nullptr), OTA_SIZE_UNKNOWN, &otaHandler);
        updateFlag = true;
    }
    if (data.length() > 0) {
        esp_ota_write(otaHandler, data.c_str(), data.length());
        if (data.length() != 512) {
            esp_ota_end(otaHandler);
            if (ESP_OK == esp_ota_set_boot_partition(esp_ota_get_next_update_partition(nullptr))) {
                LOG("ble - ota - finish");
            } else {
                LOG("ble - ota - error");
                updateFlag = false;
            }
        }
    }
    return false;
}

void BleControllerUtil::subscribe(BleServer &ble) {
    ble.on(UUID, UUID_FW_VERSION,
           std::bind(&BleControllerUtil::firmware, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED, 24);
    ble.on(UUID, UUID_HW_VERSION,
           std::bind(&BleControllerUtil::hardware, this, std::placeholders::_1),
           ESP_GATT_PERM_READ_ENCRYPTED);
    ble.on(UUID, UUID_NAME,
           (BleWHandler) std::bind(&BleControllerUtil::set_name, this, std::placeholders::_1),
           ESP_GATT_PERM_WRITE_ENCRYPTED);
    ble.on(UUID, UUID_SECRET,
           (BleWHandler) std::bind(&BleControllerUtil::set_secret, this, std::placeholders::_1),
           ESP_GATT_PERM_WRITE_ENCRYPTED);
    ble.on(UUID, UUID_FIRMWARE,
           (BleWHandler) std::bind(&BleControllerUtil::set_firmware, this, std::placeholders::_1),
           ESP_GATT_PERM_WRITE_ENCRYPTED);
}
