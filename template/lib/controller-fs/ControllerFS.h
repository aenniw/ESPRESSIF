#pragma once

#include <Service.h>
#include <EspServer.h>
#include <FileSystem.h>

class ControllerFS : public Subscriber<EspServer> {
private:
    const static uint8_t URI_OFFSET = 10;
    FileSystem &fs;
protected:
    static String getFileName(Request *request);

    void ls(Request *request) const;
    void read(Request *request) const;
    void mkdir(Request *request) const;
    void writeUrlEncoded(Request *request) const;
    void rm(Request *request) const;

public:
    explicit ControllerFS(FileSystem &fs) : fs(fs) {}

    void subscribe(EspServer &rest) const override;
};
