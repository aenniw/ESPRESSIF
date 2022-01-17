#pragma once

#include <VFS.h>

namespace ota {
    typedef enum {
        BINARY = 0,
        PATCH = 1,
    } type;
}

class Firmware {
public:
    const static String PATCH;
    const static String BINARY;
};

bool ota_flash_bin(File firmware);
bool ota_flash_patch(File firmware);
