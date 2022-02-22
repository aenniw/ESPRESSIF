#include "ControllerWiFi.h"
#include <ArduinoJson.h>
#include <EEPROM.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <detail/mimetable.h>
#endif

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 512
#endif

void wifi_connect(const WiFiMode_t mode, const char *ssid, const char *psk) {
    WiFi.mode(mode);
    if (mode == WIFI_STA) {
        WiFi.begin(ssid, psk);
        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
            log_d("Connection Failed! Waiting...");
            delay(500);
        }
        log_i("IP address: %s", WiFi.localIP().toString().c_str());
    } else if (mode == WIFI_AP) {
        WiFi.softAP(ssid, psk);
        log_i("IP address: %s", WiFi.softAPIP().toString().c_str());
    }
}

void wifi_config_reset(const WiFiMode_t mode, const char *ssid, const char *psk, const unsigned long timeout,
                       const int address) {
    EEPROM.begin(EEPROM_SIZE);

    if (!EEPROM.read(address)) {
        log_w("Restoring default WiFi config");
        wifi_connect(mode, ssid, psk);
        EEPROM.write(address, true);
        EEPROM.commit();
    } else {
        EEPROM.write(address, false);
        EEPROM.commit();
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, LOW);
        log_d("Waiting for config reset event");
        delay(timeout);
        EEPROM.write(address, true);
        EEPROM.commit();
        digitalWrite(LED_BUILTIN, HIGH);
        log_d("Continue with startup");
    }
}

const __FlashStringHelper *ControllerWiFi::get_mode(const WiFiMode_t mode) {
    switch (mode) {
        case WIFI_OFF:
            return F("OFF");
        case WIFI_STA:
            return F("STA");
        case WIFI_AP:
            return F("AP");
        case WIFI_AP_STA:
            return F("AP_STA");
#if defined(ARDUINO_ARCH_ESP8266)
        case WIFI_SHUTDOWN:
            return F("SHUTDOWN");
        case WIFI_RESUME:
            return F("RESUME");
#elif defined(ARDUINO_ARCH_ESP32)
        case WIFI_MODE_MAX:
            return F("MODE_MAX");
#endif
    }
    return F("N/A");
}

const __FlashStringHelper *ControllerWiFi::get_status(const wl_status_t status) {
    switch (status) {
        case WL_NO_SHIELD:
            return F("NO_SHIELD");
        case WL_IDLE_STATUS:
            return F("IDLE_STATUS");
        case WL_NO_SSID_AVAIL:
            return F("NO_SSID_AVAIL");
        case WL_SCAN_COMPLETED:
            return F("SCAN_COMPLETED");
        case WL_CONNECTED:
            return F("CONNECTED");
        case WL_CONNECT_FAILED:
            return F("CONNECT_FAILED");
        case WL_CONNECTION_LOST:
            return F("CONNECTION_LOST");
        case WL_DISCONNECTED:
            return F("DISCONNECTED");
    }
    return F("N/A");
}

void ControllerWiFi::get(Request *request) const {
    StaticJsonDocument<512> doc;
    doc[F("mode")] = get_mode(WiFi.getMode());

    auto sta = doc.createNestedObject(F("sta"));
    sta[F("mac")] = WiFi.macAddress();
    sta[F("address")] = WiFi.localIP().toString();
    sta[F("ssid")] = WiFi.SSID();
    sta[F("psk")] = WiFi.psk();
    sta[F("status")] = get_status(WiFi.status());

    auto ap = doc.createNestedObject(F("ap"));
    ap[F("mac")] = WiFi.softAPmacAddress();
    ap[F("address")] = WiFi.softAPIP().toString();
#ifdef ARDUINO_ARCH_ESP8266
    ap[F("ssid")] = WiFi.softAPSSID();
    ap[F("psk")] = WiFi.softAPPSK();
#endif

    String response;
    serializeJson(doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerWiFi::scan(Request *request) const {
    StaticJsonDocument<1024> doc;

    for (auto i = WiFi.scanNetworks() - 1; i >= 0; i--) {
        log_i("found AP - %s", WiFi.SSID(i).c_str());
        doc.add(WiFi.SSID(i));
    }

    String response;
    serializeJson(doc.isNull() ? doc.to<JsonArray>() : doc, response);
    request->send(200, mimeType(mime::json), response);
}

void ControllerWiFi::sta_connect(RestRequest *request) const {
    auto arg = F("plain"), arg_ssid = F("ssid"), arg_pass = F("psk");

    StaticJsonDocument<256> doc;
    if (request->hasArg(arg) &&
        !deserializeJson(doc, request->arg(arg)) &&
        !doc[arg_ssid].isNull() &&
        WiFi.disconnect() &&
        WiFi.begin(doc[arg_ssid].as<const char *>(), doc[arg_pass] | "",
                   0, nullptr, doc[F("connect")] | true) == WL_CONNECTED)
        request->send(200);
    else
        request->send(400);
}

void ControllerWiFi::sta_disconnect(Request *request) const {
    request->send(WiFi.disconnect(true) ? 200 : 400);
}

void ControllerWiFi::ap_connect(RestRequest *request) const {
    auto arg = F("plain"), arg_ssid = F("ssid"), arg_pass = F("psk");

    StaticJsonDocument<256> doc;
    if (request->hasArg(arg) &&
        !deserializeJson(doc, request->arg(arg)) &&
        !doc[arg_ssid].isNull() &&
        WiFi.softAP(doc[arg_ssid].as<const char *>(), doc[arg_pass] | "",
                    0, doc[F("hidden")] | false))
        request->send(200);
    else
        request->send(400);
}

void ControllerWiFi::ap_disconnect(Request *request) const {
    request->send(WiFi.softAPdisconnect(true) ? 200 : 400);
}

void ControllerWiFi::begin() {
    WiFi.persistent(true);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin();
}

void ControllerWiFi::subscribe(EspServer &rest) {
    rest.on(HTTP_GET, Uri(F("/api/v1/wifi")),
            std::bind(&ControllerWiFi::get, this, std::placeholders::_1));
    rest.on(HTTP_GET, Uri(F("/api/v1/wifi/scan")),
            std::bind(&ControllerWiFi::scan, this, std::placeholders::_1));
    rest.on(HTTP_POST, Uri(F("/api/v1/wifi/sta")),
            std::bind(&ControllerWiFi::sta_connect, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, Uri(F("/api/v1/wifi/sta")),
            std::bind(&ControllerWiFi::sta_disconnect, this, std::placeholders::_1));
    rest.on(HTTP_POST, Uri(F("/api/v1/wifi/ap")),
            std::bind(&ControllerWiFi::ap_connect, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, Uri(F("/api/v1/wifi/ap")),
            std::bind(&ControllerWiFi::ap_disconnect, this, std::placeholders::_1));
}
