#pragma once

#include <commons.h>
#include <EspServer.h>
#include <EspWebSocket.h>
#include <FileSystem.h>

namespace gpio {
    enum mode {
        NONE = -1,
        IN = INPUT,
        OUT = OUTPUT
    };
    enum type {
        INVALID = -1, LINEAR, DIGITAL
    };
}

class ControllerGPIO : public Subscriber<EspServer>, public Subscriber<EspWebSocket>, public Service {
private:
    const String base_path;
    FileSystem &fs;
private:
    gpio::mode get_mode(const String &s) const;
    gpio::type get_type(const String &s) const;
protected:
    void list(Request *request) const;
    void info(Request *request) const;
    void read(Request *request) const;
    void write(Request *request) const;
    void toggle(Request *request) const;
    void mode(Request *request) const;
    void rm(Request *request) const;

public:
    explicit ControllerGPIO(FileSystem &fs) : base_path(F("/gpio/")), fs(fs) {}

    void begin() override;
    void subscribe(EspServer &rest) override;
    void subscribe(EspWebSocket &ws)  override;
    void cycle() override {};

};