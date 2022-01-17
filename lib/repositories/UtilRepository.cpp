#include "UtilRepository.h"

const String UtilRepository::BLE_RESET_HANDLE = "/ble-reset";

std::string UtilRepository::get_name(const std::string &fallback, bool init) {
    std::string name = fallback;
    auto exists = fs.read(base_path + F("name"), [&](File &f) {
        name = std::string(f.readString().c_str());
    });
    LOG("ble - name %d %d - %s", exists, init, name.c_str());
    if (!exists && init) {
        this->set_name(fallback);
    }
    return name;
}

void UtilRepository::set_name(std::string n) {
    fs.write(base_path + F("name"), [&](File &f) {
        f.print(n.c_str());
    });
}

uint32_t UtilRepository::get_secret(uint32_t fallback) const {
    uint32_t secret = fallback;
    fs.read(base_path + F("secret"), [&](File &f) {
        f.read(reinterpret_cast<uint8_t *>(&secret), 4);
    });
    LOG("ble - secret %d", fallback);
    return secret;
}

void UtilRepository::set_secret(uint32_t s) {
    fs.write(base_path + F("secret"), [&](File &f) {
        f.write(reinterpret_cast<uint8_t *>(&s), 4);
    });
    fs.rm(BLE_RESET_HANDLE);
}
