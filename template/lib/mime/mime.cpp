#include <detail/mimetable.h>
#include "mime.h"

String mimeType(const String &p) {
#if defined(ARDUINO_ARCH_ESP8266)
    return mime::getContentType(p);
#elif defined(ARDUINO_ARCH_ESP32)
    using mime::mimeTable;
    char buff[sizeof(mimeTable[0].mimeType)];
    // Check all entries but last one for match, return if found
    for (size_t i=0; i < sizeof(mimeTable)/sizeof(mimeTable[0])-1; i++) {
        strcpy_P(buff, mimeTable[i].endsWith);
        if (p.endsWith(buff)) {
            strcpy_P(buff, mimeTable[i].mimeType);
            return String(buff);
        }
    }
    // Fall-through and just return default type
    strcpy_P(buff, mimeTable[sizeof(mimeTable)/sizeof(mimeTable[0])-1].mimeType);
    return String(buff);
#endif
}

const char *mimeType(uint8_t t) {
    return mime::mimeTable[t].mimeType;
}
