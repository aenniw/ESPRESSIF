#include "FileSystem.h"

void FileSystem::begin() {
    if (!VFS.begin()) {
        LOG("FS failed to initialize format");
        if (formatOnError) {
            LOG("Formating FS");
            VFS.format();
        }
        if (restartOnError) {
            LOG("Restarting");
            ESP.restart();
        }
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
        LOG("Failed to open %s", path.c_str());
    }
    return false;
}

bool FileSystem::read(const String &path, const FileHandler &h) {
    File file = fs.open(path, "r");
    if (file) {
        h(file);
        file.close();
        return true;
    } else {
        LOG("Failed to open %s", path.c_str());
    }
    return false;
}

bool FileSystem::rm(const String &path) {
    if (!fs.exists(path))
        return false;
    ls(path, [&](const String &n, const bool dir) {
        rm(path + (dir ? n + F("/") : n));
    });
    return fs.remove(path) || !fs.exists(path);;
}

bool FileSystem::mkdir(const String &path) {
    return fs.mkdir(path);
}

bool FileSystem::mv(const String &s, const String &d) {
    return fs.rename(s, d);
}
