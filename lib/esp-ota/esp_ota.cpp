#include "esp_ota.h"
#include <detools.h>
#include <esp_ota_ops.h>

const String Firmware::PATCH = "/firmware.patch";
const String Firmware::BINARY = "/firmware.bin";

bool ota_flash_bin(File firmware) {
    if (!firmware) {
        return false;
    }

    esp_ota_handle_t handle;
    esp_err_t status = ESP_OK;
    const esp_partition_t *dst = esp_ota_get_next_update_partition(nullptr);
    status = esp_ota_begin(dst, OTA_SIZE_UNKNOWN, &handle);

    while (firmware.available() && status == ESP_OK) {
        uint8_t data[64] = {0u},
                len = firmware.read(data, 64);
        status = esp_ota_write(handle, data, len);
    }

    if (status == ESP_OK) {
        esp_ota_end(handle);
        return esp_ota_set_boot_partition(dst) == ESP_OK;
    }
    return false;
}

typedef struct {
    size_t part_offset;
    const esp_partition_t *part;
    File patch;
    esp_ota_handle_t handle;
} dtools_param;

static int partition_read(void *arg_p, uint8_t *buf_p, size_t size) {
    auto params = (dtools_param *) arg_p;
    if (esp_partition_read(params->part, params->part_offset, buf_p, size) != ESP_OK) {
        return DETOOLS_INTERNAL_ERROR;
    }

    params->part_offset += size;
    return DETOOLS_OK;
}

static int partition_seek(void *arg_p, int offset) {
    auto params = (dtools_param *) arg_p;
    params->part_offset += offset;
    return DETOOLS_OK;
}

static int ota_write(void *arg_p, const uint8_t *buf_p, size_t size) {
    auto params = (dtools_param *) arg_p;
    if (esp_ota_write(params->handle, buf_p, size) != ESP_OK) {
        return DETOOLS_INTERNAL_ERROR;
    }
    return DETOOLS_OK;
}

static int fs_read(void *arg_p, uint8_t *buf_p, size_t size) {
    auto params = (dtools_param *) arg_p;
    if (params->patch.read(buf_p, size) != size) {
        return DETOOLS_INTERNAL_ERROR;
    }
    return DETOOLS_OK;
}

bool ota_flash_patch(File firmware) {
    if (!firmware) {
        return false;
    }

    dtools_param params = {
            .part_offset = 0,
            .part = esp_ota_get_running_partition(),
            .patch = firmware,
            .handle = 0
    };

    esp_err_t status = ESP_OK;
    const esp_partition_t *dst = esp_ota_get_next_update_partition(nullptr);
    status = esp_ota_begin(dst, OTA_SIZE_UNKNOWN, &(params.handle));

    if (status == ESP_OK) {
        status = detools_apply_patch_callbacks(
                partition_read, partition_seek, fs_read, firmware.size(), ota_write, &params
        ) > 0 ? ESP_OK : ESP_FAIL;
    }

    if (status == ESP_OK) {
        return esp_ota_set_boot_partition(dst) == ESP_OK;
    }
    return false;
}
