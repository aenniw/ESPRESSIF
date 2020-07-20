#pragma once

#include <Service.h>
#include <EspServer.h>
#include <FileSystem.h>

class ControllerFS : public Subscriber<EspServer> {
private:
    const static uint8_t URI_OFFSET = 10;
    FileSystem &fs;
protected:
    static String getFileName(RestRequest *request);

    void ls(RestRequest *request) const;
    void read(RestRequest *request) const;
    void mkdir(RestRequest *request) const;
    void writeUrlEncoded(RestRequest *request) const;
    void rm(RestRequest *request) const;

public:
    explicit ControllerFS(FileSystem &fs) : fs(fs) {}

    void subscribe(EspServer &rest) const override;
};
