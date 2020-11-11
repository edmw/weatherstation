#ifndef __FILES_H__
#define __FILES_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Operating Support:
// Class to manage access to the Wear-leveled SPI flash file system (SPIFFS).
// This is a no-op if not run on ESP8266. Support for ESP32 pending.
///////////////////////////////////////////////////////////////////////////////////////////////////

class Files {
public:
    Files(void);

    // Begin managing the filesystem. Must be called before any other method.
    bool begin(void);

    // Saves the given string into a file with the given name.
    void save(String filename, String string);

    // Loads the content of the file with the given name as String.
    String load(String filename);

    // Tests if a file with the given name exists.
    bool exists(String filename);

private:
};

#endif
