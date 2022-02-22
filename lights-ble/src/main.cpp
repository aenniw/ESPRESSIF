#include <configuration.h>

#include <Arduino.h>
#include <commons.h>
#include <FileSystem.h>
#include <Pixels.h>
#include <BleServer.h>
#include <BleControllerUtil.h>
#include <BleControllerPixels.h>
#include <Haptic.h>

FileSystem fileSystem(true, true, true);
UtilRepository utilRepository(fileSystem);
BleControllerUtil controllerUtil(FW_VERSION, HW_VERSION, utilRepository);
Haptic haptics(GPIO_NUM_33, GPIO_NUM_0, {GPIO_NUM_12, GPIO_NUM_27, GPIO_NUM_32});

std::vector<PixelsSupplier> PixelsStrands = {
        NeoPixelsRtm(GPIO_NUM_21, 0),
#if PIXELS_STRANDS > 1
        NeoPixelsRtm(GPIO_NUM_22, 1),
#endif
#if PIXELS_STRANDS > 2
        NeoPixelsRtm(GPIO_NUM_23, 2)
#endif
};

std::vector<Service *> services;

// cppcheck-suppress unusedFunction
void setup() {
    fileSystem.begin();

    log_i("setup");
    auto bleServer = new BleServer(
            utilRepository.get_name(BLE_NAME, true),
            BLE_MANUFACTURER,
            utilRepository.get_secret(BLE_SECRET)
    );
    bleServer->serve(&controllerUtil);

    uint32_t length = 0;
    const uint16_t current = utilRepository.get_power(PIXELS_CURRENT, true);

    PixelsRepository *repositories[PixelsStrands.size()];
    for (size_t i = 0; i < PixelsStrands.size(); i++) {
        repositories[i] = new PixelsRepository(fileSystem, i);
        length += repositories[i]->get_length(PIXELS_LEN, true);
    }

    const auto power = pixel::power_scale(current, PIXELS_POTENTIAL, length);
    for (size_t i = 0; i < PixelsStrands.size(); i++) {
        auto pixels = PixelsStrands[i](repositories[i]->get_length(), power);
        services.push_back(pixels);

        auto controller = new BleControllerPixels(*pixels, *repositories[i]);
        bleServer->serve(controller);
        haptics.serve(controller);
    }

    services.push_back(bleServer);
    services.push_back(&haptics);

    log_i("initialize");
    services.shrink_to_fit();
    for (auto &service: services)
        service->begin();

    if (!fileSystem.exists(UtilRepository::BLE_RESET_HANDLE)) {
        BleServer::rm_bonds();
        fileSystem.touch(UtilRepository::BLE_RESET_HANDLE);
    }
    log_i("initialized");
}

// cppcheck-suppress unusedFunction
void loop() {
    for (auto &service: services)
        service->cycle();
}