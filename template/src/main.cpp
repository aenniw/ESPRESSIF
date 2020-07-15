#include <configuration.h>

#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include <Service.h>
#include <FileSystem.h>
#include <EspServer.h>
#include <ControllerGPIO.h>
#include <ControllerFS.h>

void wifiConnect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        LOG("Connection Failed! Waiting...");
        delay(500);
    }

    LOG("Started.");
    LOG("IP address: %s", WiFi.localIP().toString().c_str());
}

FileSystem fileSystem;
ControllerFS controllerFS(fileSystem);
ControllerGPIO controllerGPIO(fileSystem);
EspServer rest(80);

std::vector<Service *> services{&fileSystem, &controllerGPIO, &rest};

void setup() {
    LOG_INIT(Serial.begin(MONITOR_SPEED), &Serial);
    LOG("starting");

    wifiConnect();

    rest.serve(controllerGPIO).serve(controllerFS);
    rest.serveStatic("/", VFS, "/www/", "max-age=3600");

    for (auto &service : services)
        service->begin();
}

void loop() {
    for (auto &service : services)
        service->cycle();
}