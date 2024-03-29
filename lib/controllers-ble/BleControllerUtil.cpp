#include <BleControllerUtil.h>
#include <esp_ota.h>

void BleControllerUtil::power(BLECharacteristic &c) const {
    auto p = repository.get_power();
    LOG("ble - power %d", p);
    c.setValue(p);
}

bool BleControllerUtil::set_power(BLECharacteristic &c) {
    repository.set_power(c.getValue<uint16_t>());
    return true;
}

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
    if (pin.length() == 6) {
        repository.set_secret(
                (uint32_t) strtol(pin.c_str(), nullptr, 10)
        );
        VFS.remove(UtilRepository::BLE_RESET_HANDLE);
    }
    return false;
}

bool BleControllerUtil::set_firmware(BLECharacteristic &c) {
    auto msg = c.getValue();
    auto data = (uint8_t *) msg.data();
    size_t offset = 0, len = msg.length();

    if (!ota_streaming && len >= sizeof(ble_ota_header_t)) {
        ota_header = reinterpret_cast<ble_ota_header_t *>(data)[0];
        offset = sizeof(ble_ota_header_t);
        ota_streaming = true;
        md5.begin();

        VFS.remove(Firmware::PATCH);
        VFS.remove(Firmware::BINARY);
        LOG("ble - ota - start %d %d, %llx%llx", ota_header.length, ota_header.type,
            ota_header.md5_le, ota_header.md5_hi);
    }

    auto indicate = false;
    if (ota_streaming) {
        const auto type = (ota::type) ota_header.type;
        size_t dataLen = len - offset, uploaded_size;
        String firmware = type == ota::BINARY ?
                          Firmware::BINARY : Firmware::PATCH;

        const auto exists = VFS.exists(firmware);
        File f = VFS.open(firmware, exists ? FILE_APPEND : FILE_WRITE);
        uploaded_size = (exists ? f.size() : 0u) + f.write(data + offset, dataLen);
        md5.add(data + offset, dataLen);
        f.close();

        LOG("ble - ota - uploaded %d/%d - %d", uploaded_size, ota_header.length, dataLen);

        if (uploaded_size >= ota_header.length) {
            uint8_t hash[16] = {0u};
            md5.calculate();
            md5.getBytes(hash);

            bool checksum = true;
            for (uint8_t i = 0; i < 8; i++) {
                if (hash[7 - i] != ota_header.md5_le8[i] ||
                    hash[15 - i] != ota_header.md5_hi8[i]) {
                    checksum = false;
                }
            }

            if (uploaded_size == ota_header.length && checksum) {
                LOG("ble - ota - flashing");
                f = VFS.open(firmware, FILE_READ);
                indicate = type == ota::BINARY ?
                           ota_flash_bin(f) :
                           ota_flash_patch(f);
                f.close();
                LOG("ble - ota - flashed %d", indicate);
            } else {
                LOG("ble - ota - checksum invalid %s", md5.toString().c_str());
            }
            LOG("ble - ota - cleanup");
            VFS.remove(firmware);
            ota_streaming = false;
        }
    }

    return indicate;
}

const NimBLEUUID  BleControllerUtil::UUID = fullUUID(0xab5fa770u);
const NimBLEUUID  BleControllerUtil::UUID_POWER = fullUUID(0xb534fb4eu);
const NimBLEUUID  BleControllerUtil::UUID_NAME = fullUUID(0x02f3714eu);
const NimBLEUUID  BleControllerUtil::UUID_SECRET = fullUUID(0x02f3724eu);
const NimBLEUUID  BleControllerUtil::UUID_FIRMWARE = fullUUID(0x03f3704eu);
const NimBLEUUID  BleControllerUtil::UUID_FW_VERSION = fullUUID(0x02f3704eu);
const NimBLEUUID  BleControllerUtil::UUID_HW_VERSION = fullUUID(0x04f3704eu);

void BleControllerUtil::subscribe(BleServer &ble) {
    ble.on(UUID, UUID_POWER,
           std::bind(&BleControllerUtil::power, this, std::placeholders::_1),
           std::bind(&BleControllerUtil::set_power, this, std::placeholders::_1), 30);
    ble.on(UUID, UUID_FW_VERSION, std::bind(&BleControllerUtil::firmware, this, std::placeholders::_1));
    ble.on(UUID, UUID_HW_VERSION, std::bind(&BleControllerUtil::hardware, this, std::placeholders::_1));
    ble.on(UUID, UUID_NAME, (BleWHandler) std::bind(&BleControllerUtil::set_name, this, std::placeholders::_1));
    ble.on(UUID, UUID_SECRET, (BleWHandler) std::bind(&BleControllerUtil::set_secret, this, std::placeholders::_1));
    ble.on(UUID, UUID_FIRMWARE, (BleWHandler) std::bind(&BleControllerUtil::set_firmware, this, std::placeholders::_1));
}