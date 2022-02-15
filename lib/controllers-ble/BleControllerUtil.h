#pragma once

#include <commons.h>
#include <BleServer.h>
#include <UtilRepository.h>
#include <MD5Builder.h>
#include <esp_ota_ops.h>

typedef struct {
    uint32_t length;
    struct {
        uint32_t type: 8;
    };
    union {
        uint64_t md5_hi;
        uint8_t md5_hi8[8];
    };
    union {
        uint64_t md5_le;
        uint8_t md5_le8[8];
    };
} ble_ota_header_t;

class BleControllerUtil : public Subscriber<BleServer> {
public:
    static const NimBLEUUID UUID, UUID_POWER, UUID_NAME, UUID_SECRET, UUID_FIRMWARE, UUID_FW_VERSION, UUID_HW_VERSION;
private:
    std::string fw_version, hw_version;
    UtilRepository &repository;

    MD5Builder md5;
    bool ota_streaming = false;
    ble_ota_header_t ota_header = {};
protected:
    void power(BLECharacteristic &c) const;
    bool set_power(BLECharacteristic &c);
    bool set_name(BLECharacteristic &c);
    bool set_secret(BLECharacteristic &c);
    void hardware(BLECharacteristic &c) const;
    void firmware(BLECharacteristic &c) const;
    bool set_firmware(BLECharacteristic &c);
public:
    explicit BleControllerUtil(std::string fw_version, std::string hw_version, UtilRepository &repository) :
            fw_version(std::move(fw_version)), hw_version(std::move(hw_version)), repository(repository) {}

    void subscribe(BleServer &ble) override;;
};
