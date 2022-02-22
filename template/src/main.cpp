#include <configuration.h>

#include <Arduino.h>
#include <commons.h>
#include <FileSystem.h>
#include <Pixels.h>
#include <EspServer.h>
#include <EspWebSocket.h>
#include <ControllerGPIO.h>
#include <ControllerFS.h>
#include <ControllerBearer.h>
#include <ControllerWiFi.h>
#include <ControllerPixels.h>

FileSystem fileSystem;
NeoPixels<NeoGrbFeature, Neo800KbpsMethod, NeoGammaTableMethod> pixels(PIXELS_LEN, PIXELS_PIN);

EspServer rest(80, AUTH_USER, AUTH_PASS);
EspWebSocket webSocket(81, AUTH_PASS);

ControllerWiFi controllerWiFi;
ControllerFS controllerFS(fileSystem);
ControllerGPIO controllerGPIO(fileSystem);
ControllerBearer controllerBearer(fileSystem, true, 20);
ControllerPixels controllerPixels(fileSystem, pixels);

std::vector<Service *> services{
        &fileSystem, &pixels,
        &controllerGPIO, &controllerBearer, &controllerWiFi, &controllerPixels,
        &rest, &webSocket
};

// cppcheck-suppress unusedFunction
void setup() {
    log_init(&DEBUG_ESP_PORT, MONITOR_SPEED);
    log_i("initializing");

    wifi_config_reset(WIFI_MODE, WIFI_SSID, WIFI_PSK);

    rest.setBearerValidator(&controllerBearer);
    rest.serve(controllerGPIO)
            .serve(controllerFS)
            .serve(controllerBearer)
            .serve(controllerWiFi)
            .serve(controllerPixels);
    rest.serveStatic("/", VFS, "/www/", "max-age=3600");

    webSocket.serve(controllerGPIO);

    for (auto &service : services)
        service->begin();
    log_i("initialized");
}

// cppcheck-suppress unusedFunction
void loop() {
    for (auto &service : services)
        service->cycle();
}