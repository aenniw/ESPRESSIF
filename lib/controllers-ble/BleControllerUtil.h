#pragma once

#include <commons.h>
#include <BleServer.h>
#include <UtilRepository.h>
#include <esp_ota_ops.h>

class BleControllerUtil : public Subscriber<BleServer> {
public:
    static const uint32_t UUID = 0xab5fa770,
            UUID_NAME = 0x02f3714e,
            UUID_SECRET = 0x02f3724e,
            UUID_FIRMWARE = 0x03f3704e,
            UUID_FW_VERSION = 0x02f3704e,
            UUID_HW_VERSION = 0x04f3704e;
private:
    esp_ota_handle_t otaHandler = 0;
    bool updateFlag = false;

    std::string fw_version, hw_version;
    UtilRepository &repository;
protected:
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
