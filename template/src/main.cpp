#include <configuration.h>

#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include <Service.h>
#include <EspServer.h>
#include <ControllerGPIO.h>

void wifiConnect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        LOG("Connection Failed! Waiting...");
        delay(5000);
    }

    LOG("Started.");
    LOG("IP address: %s", WiFi.localIP().toString().c_str());
}

ControllerGPIO controllerGPIO(VFS);
EspServer rest(80);

std::vector<Service *> services{&controllerGPIO, &rest};

void setup() {
    LOG_INIT(Serial.begin(MONITOR_SPEED), &Serial);
    LOG("starting");

    wifiConnect();

    LOG("Mount LittleFS");
    if (!VFS.begin()) {
        LOG("LittleFS format");
        VFS.format();
        ESP.restart();
    }

    rest.serve(controllerGPIO);
    rest.serveStatic("/", VFS, "/www/", "max-age=3600");

    for (auto &service : services)
        service->begin();
}

void loop() {
    for (auto &service : services)
        service->cycle();
}