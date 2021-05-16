#include <configuration.h>

#include <Arduino.h>
#include <commons.h>
#include <FileSystem.h>
#include <Pixels.h>
#include <BleServer.h>
#include <BleControllerPixels.h>

FileSystem fileSystem;
NeoPixels<NeoGrbFeature, Neo800KbpsMethod> pixels(PIXELS_LEN, PIXELS_PIN);

BleServer ble(BLE_NAME, BLE_PIN);
BleControllerPixels pixelsBle(fileSystem, pixels);
std::vector<Service *> services{&ble, &pixels, &pixelsBle};

// cppcheck-suppress unusedFunction
void setup() {
    LOG_INIT(&DEBUG_ESP_PORT, MONITOR_SPEED);
    LOG("initializing");

    ble.serve(pixelsBle);

    for (auto &service : services)
        service->begin();
    LOG("initialized");
}

// cppcheck-suppress unusedFunction
void loop() {
    for (auto &service : services)
        service->cycle();
}