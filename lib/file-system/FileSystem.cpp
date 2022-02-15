#include "FileSystem.h"

void FileSystem::factory_reset(uint32_t wait) {
    auto path = F("/factory-reset");
#ifdef LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
#endif
    if (rm(path)) {
        LOG("fs - factory reset.");
        format();
        ESP.restart();
    } else {
        touch(path);
        LOG("fs - waiting for config reset event.");
        delay(wait);
        rm(path);
#ifdef LED_BUILTIN
        digitalWrite(LED_BUILTIN, LOW);
#endif
    }
}

void FileSystem::begin() {
    if (!VFS.begin()) {
        LOG("fs - failed to initialize");
        if (formatOnError) {
            LOG("fs - formatting");
            VFS.format();
        }
        if (restartOnError) {
            LOG("fs - restarting");
            ESP.restart();
        }
    } else if (formatOnReset) {
        factory_reset(2000);
    }
}

#if defined(ARDUINO_ARCH_ESP8266)
bool FileSystem::ls(const String &dirname, const LsHandler &h) {
    Dir root = fs.openDir(dirname);
    if (!dirname.endsWith(F("/")))
        return false;
    bool nonEmpty = false;
    while (root.next()) {
        h(root.fileName(), root.isDirectory());
        nonEmpty |= true;
    }
    return nonEmpty;
}
#elif defined(ARDUINO_ARCH_ESP32)
bool FileSystem::ls(const String &dirname, const LsHandler &h) {
    File root = fs.open(dirname);
    if (!root || !root.isDirectory()) {
        return false;
    }
    File file = root.openNextFile();
    bool nonEmpty = false;
    while (file) {
        h(String(file.name()), file.isDirectory());
        file = root.openNextFile();
        nonEmpty |= true;
    }
    file.close();
    return nonEmpty;
}
#endif

bool FileSystem::write(const String &path, const FileHandler &h, const char *mode) {
    File file = fs.open(path, mode);
    if (file) {
        h(file);
        file.close();
        return true;
    } else {
        LOG("fs - failed to open %s", path.c_str());
    }
    return false;
}

bool FileSystem::read(const String &path, const FileHandler &h) {
    File file = fs.open(path, "r");
    if (file && file.available() > 0) {
        h(file);
        file.close();
        return true;
    } else {
        LOG("fs - failed to open %s", path.c_str());
    }
    return false;
}

bool FileSystem::rm(const String &path) {
    if (!exists(path))
        return false;
    ls(path, [&](const String &n, const bool dir) {
        rm(path + (dir ? n + F("/") : n));
    });
    return fs.remove(path) || !exists(path);;
}

bool FileSystem::mkdir(const String &path) {
    return fs.mkdir(path);
}

bool FileSystem::mv(const String &s, const String &d) {
    return fs.rename(s, d);
}

bool FileSystem::exists(const String &path) {
    return fs.exists(path);
}

bool FileSystem::touch(const String &path) {
    return write(path, [&](File &f) { f.write(1); });
}

void FileSystem::format() {
    VFS.format();
}
