#pragma once

#include <commons.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <LittleFS.h>
#define VFS LittleFS
#elif defined(ARDUINO_ARCH_ESP32)
#include <SPIFFS.h>
#define VFS SPIFFS
#endif

typedef std::function<void(File &f)> FileHandler;
typedef std::function<void(const String &n, const bool dir)> LsHandler;

class FileSystem : public Service {
private:
    const bool formatOnReset, formatOnError, restartOnError;
    FS &fs = VFS;
protected:
    void factory_reset(uint32_t wait);
public:
    explicit FileSystem(bool formatOnReset = false, bool formatOnError = false, bool restartOnError = false) :
            formatOnReset(formatOnReset), formatOnError(formatOnError), restartOnError(restartOnError) {}

    void begin() override;

    void format();
    bool ls(const String &dirname, const LsHandler &h);
    bool write(const String &path, const FileHandler &h, const char *mode = "w");
    bool read(const String &path, const FileHandler &h);
    bool rm(const String &path);
    bool mv(const String &s, const String &d);
    bool mkdir(const String &path);
    bool exists(const String &path);
    bool touch(const String &path);

    void cycle() override {};
};