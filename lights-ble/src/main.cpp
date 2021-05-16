#include <configuration.h>

#include <Arduino.h>
#include <OneButton.h>
#include <commons.h>
#include <FileSystem.h>
#include <Pixels.h>
#include <BleServer.h>
#include <BleControllerUtil.h>
#include <BleControllerPixels.h>

const auto BLE_RESET_HANDLE = F("/ble-reset");

OneButton rightButton(GPIO_NUM_16, true),
        leftButton(GPIO_NUM_17, true);

FileSystem fileSystem(true, true, true);
PixelsRepository pixelRepository(fileSystem);
UtilRepository utilRepository(fileSystem);

std::vector<Service *> services;
std::vector<OneButton *> buttons = {&leftButton, &rightButton};

// cppcheck-suppress unusedFunction
void setup() {
    LOG_INIT(&DEBUG_ESP_PORT, MONITOR_SPEED);
    fileSystem.begin();

    auto pixels = new NeoPixels<NeoGrbFeature, Neo800KbpsMethod, NeoGammaTableMethod>(
            pixelRepository.get_length(PIXELS_LEN, true),
            PIXELS_PIN,
            pixelRepository.get_power(PIXELS_CURRENT, true),
            PIXELS_POTENTIAL
    );
    auto pixelsBle = new BleControllerPixels(*pixels, pixelRepository);
    auto bleServer = new BleServer(
            utilRepository.get_name(BLE_NAME, true),
            BLE_MANUFACTURER,
            utilRepository.get_secret(BLE_SECRET)
    );

    bleServer->serve(new BleControllerUtil(FW_VERSION, HW_VERSION, utilRepository));
    bleServer->serve(pixelsBle);

    services.push_back(pixels);
    services.push_back(bleServer);

    for (auto &service: services)
        service->begin();

    if (!fileSystem.exists(BLE_RESET_HANDLE)) {
        BleServer::rm_bonds();
        fileSystem.touch(BLE_RESET_HANDLE);
    }

    leftButton.attachClick([](void *param) { ((BleControllerPixels *) param)->toggle(); }, pixelsBle);
    leftButton.attachDuringLongPress([](void *param) { ((BleControllerPixels *) param)->brighten(); }, pixelsBle);
    rightButton.attachClick([](void *param) { ((BleControllerPixels *) param)->toggle_modes(); }, pixelsBle);
    rightButton.attachDuringLongPress([](void *param) { ((BleControllerPixels *) param)->speedup(); }, pixelsBle);
}

// cppcheck-suppress unusedFunction
void loop() {
    for (auto &service: services)
        service->cycle();
    for (auto &button: buttons)
        button->tick();
}