#pragma once

#include <commons.h>
#include <EspServer.h>
#include <FileSystem.h>
#include <elapsedMillis.h>

class ControllerBearer : public Service, public Subscriber<EspServer>, public Predicate<String> {
private:
    const String base_path;
    const uint8_t tokenGenerateTimeout;
    elapsedSeconds elapsed;
    FileSystem &fs;
    bool canGenerate = false;
protected:
    String generate_token();

    void add(Request *request);
    void list(Request *request) const;
    void reset(Request *request) const;

public:
    explicit ControllerBearer(FileSystem &fs, bool active = false, uint8_t tokenGenerateTimeout = 5) :
            base_path(F("/bearer/")), tokenGenerateTimeout(tokenGenerateTimeout), fs(fs), canGenerate(active) {}

    void begin() override;
    void enable();
    bool test(String &t) const override;
    void subscribe(EspServer &rest) override;
    void cycle() override;
};
