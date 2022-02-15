#pragma once

#include <commons.h>
#include <FileSystem.h>

class UtilRepository {
public:
    const static String BLE_RESET_HANDLE;
private:
    const String base_path;
    FileSystem &fs;
public:
    explicit UtilRepository(FileSystem &fs) : base_path(F("/util/")), fs(fs) {}

    uint16_t get_power(uint16_t fallback = 0, bool init = false);
    void set_power(uint16_t p);

    std::string get_name(const std::string &fallback, bool init = false);
    void set_name(std::string n);

    uint32_t get_secret(uint32_t fallback = 0) const;
    void set_secret(uint32_t s);
};

