#pragma once

#include <commons.h>
#include <EspServer.h>
#include <EspRequest.h>

void wifi_connect(WiFiMode_t mode, const char *ssid, const char *psk);
void
wifi_config_reset(WiFiMode_t mode, const char *ssid, const char *psk, unsigned long timeout = 3000, int address = 0);

class ControllerWiFi : public Service, public Subscriber<EspServer> {
private:
    static const __FlashStringHelper *get_mode(WiFiMode_t mode) ;
    static const __FlashStringHelper *get_status(wl_status_t status) ;
protected:
    void get(Request *request) const;
    void scan(Request *request) const;
    void sta_connect(RestRequest *request) const;
    void sta_disconnect(Request *request) const;
    void ap_connect(RestRequest *request) const;
    void ap_disconnect(Request *request) const;
public:

    void begin() override;
    void subscribe(EspServer &rest) override;
    void cycle() override {};
};
