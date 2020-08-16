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
    const bool formatOnError, restartOnError;
    FS &fs = VFS;
public:
    explicit FileSystem(bool formatOnError = false, bool restartOnError = false) :
            formatOnError(formatOnError), restartOnError(restartOnError) {}

    void begin() override;

    bool ls(const String &dirname, const LsHandler &h);
    bool write(const String &path, const FileHandler &h, const char *mode = "w");
    bool read(const String &path, const FileHandler &h);
    bool rm(const String &path);
    bool mv(const String &s, const String &d);
    bool mkdir(const String &path);

    void cycle() override {};
};