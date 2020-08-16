#include <configuration.h>

#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include <commons.h>
#include <FileSystem.h>
#include <EspServer.h>
#include <EspWebSocket.h>
#include <ControllerGPIO.h>
#include <ControllerFS.h>
#include <ControllerBearer.h>

void wifiConnect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
#if defined(ARDUINO_ARCH_ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
#endif
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        LOG("Connection Failed! Waiting...");
        delay(500);
    }

    LOG("Started.");
    LOG("IP address: %s", WiFi.localIP().toString().c_str());
}

EspServer rest(80, AUTH_USER, AUTH_PASS);
EspWebSocket webSocket(81, AUTH_PASS);
FileSystem fileSystem;

ControllerFS controllerFS(fileSystem);
ControllerGPIO controllerGPIO(fileSystem);
ControllerBearer controllerBearer(fileSystem, true, 20);

std::vector<Service *> services{&fileSystem, &controllerGPIO, &controllerBearer, &rest, &webSocket};

void setup() {
    LOG_INIT(&DEBUG_ESP_PORT, MONITOR_SPEED);
    LOG("initializing");

    wifiConnect();

    rest.setBearerValidator(&controllerBearer);
    rest.serve(controllerGPIO).serve(controllerFS).serve(controllerBearer);
    rest.serveStatic("/", VFS, "/www/", "max-age=3600");

    webSocket.serve(controllerGPIO);

    for (auto &service : services)
        service->begin();
    LOG("initialized");
}

void loop() {
    for (auto &service : services)
        service->cycle();
}