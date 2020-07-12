#include <ArduinoJson.h>
#include <EspServer.h>
#include <detail/mimetable.h>
#ifdef ARDUINO_ARCH_ESP32
#include <analogWrite.h>
#endif
#include "ControllerGPIO.h"

typedef std::function<void(const String &n)> FSHandler;

#if defined(ARDUINO_ARCH_ESP8266)
bool listDir(FS &fs, const String &dirname, const FSHandler &h) {
    Dir root = fs.openDir(dirname);
    bool nonEmpty = false;
    while (root.next()) {
        h(root.fileName());
        nonEmpty |= true;
    }
    return nonEmpty;
}
#elif defined(ARDUINO_ARCH_ESP32)
bool listDir(FS &fs, const String &dirname, const FSHandler &h) {
    File root = fs.open(dirname);
    if (!root || !root.isDirectory()) {
        return false;
    }
    File file = root.openNextFile();
    bool nonEmpty = false;
    while (file) {
        h(String(file.name()));
        file = root.openNextFile();
        nonEmpty |= true;
    }
    return nonEmpty;
}
#endif

static void writeFile(FS &fs, const char *type, uint8_t pin, int value) {
    String path = "/gpio/";
    path += pin;
    path += "/";
    path += type;
    LOG("Write file %s %d", path.c_str(), value);
    auto file = fs.open(path, "w");
    if (file) {
        if (!file.print(value)) {
            LOG("Failed to write %s", path.c_str());
        }
        file.close();
    } else {
        LOG("Failed to open %s", path.c_str());
    }
}

static bool readFile(FS &fs, const String &path, char *buffer, size_t length) {
    File file = fs.open(path, "r");
    if (file) {
        if (!file.readBytes(buffer, length)) {
            LOG("Failed to read %s", path.c_str());
        }
        file.close();
        return true;
    } else {
        LOG("Failed to open %s", path.c_str());
    }
    return false;
}

void ControllerGPIO::begin() {
    String path = "/gpio";
    listDir(fs, path, [&](const String &n) {
        auto pin = (uint8_t) n.toInt();

        char type[4] = {'\0'};
        char value[8] = {'\0'};
        if (readFile(fs, path + "/" + n + "/type", type, 3) &&
            readFile(fs, path + "/" + n + "/value", value, 7)) {
            LOG("writePin %d %s %s", pin, type, value);
            const auto gpio_type = (GPIO_TYPE) atoi(type);
            switch (gpio_type) {
                case ANAL:
                    analogWrite(pin, atoi(value));
                    break;
                case DIGI:
                    digitalWrite(pin, (uint8_t) atoi(value));
                    break;
            }
        }

        char mode[4] = {'\0'};
        if (readFile(fs, path + "/" + n + "/mode", mode, 3)) {
            pinMode(pin, (uint8_t) atoi(mode));
            LOG("pinMode %d %s", pin, mode);
        }
    });
}

void ControllerGPIO::subscribe(EspServer &rest) const {
    rest.on(HTTP_GET, F(R"(^\/api\/v1\/gpio$)"),
            std::bind(&ControllerGPIO::list, this, std::placeholders::_1));
    rest.on(HTTP_GET, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})$)"),
            std::bind(&ControllerGPIO::info, this, std::placeholders::_1));
    rest.on(HTTP_PUT, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})$)"),
            std::bind(&ControllerGPIO::toggle, this, std::placeholders::_1));
    rest.on(HTTP_DELETE, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})$)"),
            std::bind(&ControllerGPIO::delete_gpio, this, std::placeholders::_1));
    rest.on(HTTP_GET, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})\/(analog|digital)$)"),
            std::bind(&ControllerGPIO::read, this, std::placeholders::_1));
    rest.on(HTTP_PUT, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})\/(analog|digital)\/([0-9]{1,4})$)"),
            std::bind(&ControllerGPIO::write, this, std::placeholders::_1));
    rest.on(HTTP_POST, F(R"(^\/api\/v1\/gpio\/([0-9]{1,2})\/(input|output)$)"),
            std::bind(&ControllerGPIO::set_mode, this, std::placeholders::_1));
}

void ControllerGPIO::delete_gpio(Request *request) const {
    String path = "/gpio/";
    path += request->pathArg(0);

    if (fs.exists(path)) {
        listDir(fs, path, [&](const String &n) {
            fs.remove(path + "/" + n);
        });
        fs.rmdir(path);
        request->send(200);
    } else {
        request->send(404);
    }
}

void ControllerGPIO::set_mode(Request *request) const {
    const auto pin = (uint8_t) request->pathArg(0).toInt();
    const auto mode = (GPIO_MODE) (request->pathArg(1).length() - 5);
    pinMode(pin, mode);
    writeFile(fs, "mode", pin, mode);
    request->send(200);
}

void ControllerGPIO::toggle(Request *request) const {
    const auto pin = (uint8_t) request->pathArg(0).toInt();
    const auto value = (uint8_t) !digitalRead(pin);
    digitalWrite(pin, value);
    writeFile(fs, "value", pin, value);
    writeFile(fs, "type", pin, DIGI);
    request->send(200);
}

void ControllerGPIO::write(Request *request) const {
    const auto pin = (uint8_t) request->pathArg(0).toInt();
    const auto type = (GPIO_TYPE) request->pathArg(1).length();
    const auto value = request->pathArg(2).toInt();
    if (type == ANAL) {
        analogWrite(pin, value);
    } else if (type == DIGI) {
        digitalWrite(pin, (uint8_t) value);
    } else {
        return request->send(400);
    }
    writeFile(fs, "type", pin, type);
    writeFile(fs, "value", pin, value);
    request->send(200);
}

void ControllerGPIO::read(Request *request) const {
    const auto pin = (uint8_t) request->pathArg(0).toInt();
    const auto mode = (GPIO_TYPE) request->pathArg(1).length();
    auto value = 0;
    if (mode == ANAL) {
        value = analogRead(pin);
    } else if (mode == DIGI) {
        value = digitalRead(pin);
    } else {
        return request->send(400);
    }

    StaticJsonDocument<32> doc;
    doc[F("value")] = value;

    String response;
    serializeJson(doc, response);
    request->send(200, mineType(mime::json), response);
}

void ControllerGPIO::info(Request *request) const {
    StaticJsonDocument<128> doc;
    String path = "/gpio/";
    path += request->pathArg(0);
    path += "/";

    listDir(fs, path, [&](const String &n) {
        char buffer[8] = {'\0'};
        readFile(fs, path + "/" + n, buffer, 15);
        doc[n] = buffer;
    });

    String response;
    if (doc.isNull()) {
        request->send(404);
    } else {
        serializeJson(doc, response);
        request->send(200, mineType(mime::json), response);
    }
}

void ControllerGPIO::list(Request *request) const {
    StaticJsonDocument<64> doc;

    listDir(fs, "/gpio/", [&](const String &n) {
        doc.add(n.toInt());
    });

    String response;
    serializeJson(doc.isNull() ? doc.to<JsonArray>() : doc, response);
    request->send(200, mineType(mime::json), response);
}
