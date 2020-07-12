#pragma once

#include <FS.h>
#include <Service.h>
#include <EspServer.h>

typedef enum {
    IN = INPUT,
    OUT = OUTPUT
} GPIO_MODE;

typedef enum {
    ANAL = 6,
    DIGI = 7
} GPIO_TYPE;

class ControllerGPIO : public Subscriber<EspServer>, public Service {
private:
    FS &fs;
protected:
    void list(Request *request) const;

    void info(Request *request) const;

    void read(Request *request) const;

    void write(Request *request) const;

    void toggle(Request *request) const;

    void set_mode(Request *request) const;

    void delete_gpio(Request *request) const;

public:
    explicit ControllerGPIO(FS &fs) : fs(fs) {}

    void begin() override;;

    void subscribe(EspServer &rest) const override;

    void cycle() override {};

};