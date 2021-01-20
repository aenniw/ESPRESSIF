#include <configuration.h>

#include <Arduino.h>
#include <commons.h>
#include <FileSystem.h>
#include <EspServer.h>
#include <EspWebSocket.h>
#include <ControllerGPIO.h>
#include <ControllerFS.h>
#include <ControllerBearer.h>
#include <ControllerWiFi.h>

EspServer rest(80, AUTH_USER, AUTH_PASS);
EspWebSocket webSocket(81, AUTH_PASS);
FileSystem fileSystem;

ControllerWiFi controllerWiFi;
ControllerFS controllerFS(fileSystem);
ControllerGPIO controllerGPIO(fileSystem);
ControllerBearer controllerBearer(fileSystem, true, 20);

std::vector<Service *> services{&fileSystem, &controllerGPIO, &controllerBearer, &controllerWiFi, &rest, &webSocket};

// cppcheck-suppress unusedFunction
void setup() {
    LOG_INIT(&DEBUG_ESP_PORT, MONITOR_SPEED);
    LOG("initializing");

    wifi_config_reset(WIFI_MODE, WIFI_SSID, WIFI_PSK);

    rest.setBearerValidator(&controllerBearer);
    rest.serve(controllerGPIO).serve(controllerFS).serve(controllerBearer).serve(controllerWiFi);
    rest.serveStatic("/", VFS, "/www/", "max-age=3600");

    webSocket.serve(controllerGPIO);

    for (auto &service : services)
        service->begin();
    LOG("initialized");
}

// cppcheck-suppress unusedFunction
void loop() {
    for (auto &service : services)
        service->cycle();
}