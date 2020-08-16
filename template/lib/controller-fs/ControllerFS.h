#pragma once

#include <commons.h>
#include <EspServer.h>
#include <FileSystem.h>

class ControllerFS : public Subscriber<EspServer> {
private:
    const static uint8_t URI_OFFSET = 10;
    FileSystem &fs;
private:
    static String getFileName(RestRequest *request, unsigned int offset = 0);
protected:
    void ls(RestRequest *request) const;
    void read(RestRequest *request) const;
    void mkdir(RestRequest *request) const;
    void writeUrlEncoded(RestRequest *request) const;
    void rm(RestRequest *request) const;

public:
    explicit ControllerFS(FileSystem &fs) : fs(fs) {}

    void subscribe(EspServer &rest) override;
};
