#include <Arduino.h>

#ifdef ESP8266
#include <FS.h>
#endif

#include "Files.h"

#include "Notification.h"

extern const bool PRODUCTION;
extern Notification notification;

///////////////////////////////////////////////////////////////////////////////////////////////////

Files::Files(void) {
}

bool Files::begin(void) {
    #ifdef ESP8266
    if (SPIFFS.begin()) {
        if (!PRODUCTION) {
            // Development: print statistics
            FSInfo fs_info;
            SPIFFS.info(fs_info);
            notification.info(F("*FILES: Total bytes: "), fs_info.totalBytes);
            notification.info(F("*FILES: Used bytes: "), fs_info.usedBytes);
        }
        return true;
    }
    return false;
    #else
    return true;
    #endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Files::save(String filename, String string) {
    #ifdef ESP8266
    File outfile = SPIFFS.open(String(filename + ".dat"), "w");
    if (outfile) {
        outfile.println(String(string + String('\r')));
        outfile.close();
    }
    else {
        notification.warn(F("Failed to write to file:"), filename);
    }
    #endif
}

String Files::load(String filename) {
    #ifdef ESP8266
    String string;
    File infile = SPIFFS.open(String(filename + ".dat"), "r");
    if (infile) {
        string = infile.readStringUntil('\r');
        string.replace("\n", "");
        infile.close();
        return string;
    }
    else {
        notification.warn(F("Failed to read from file:"), filename);
    }
    #endif
}

bool Files::exists(String filename) {
  #ifdef ESP8266
  return SPIFFS.exists(String(filename + ".dat"));
  #else
  return false;
  #endif
}
