#ifndef __VALUES_H__
#define __VALUES_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Operating Support:
// Class to manage values in a key-value-store like manner on top of the files manager.
// This is a no-op if not run on ESP8266. Support for ESP32 pending (see Files).
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Files.h"

class Values {
public:
    // Enumeration of possible value keys.
    enum key {
      influx_secret = 0
    };

    Values(void);

    // Begin managing the values with the given files manager.
    // Must be called before any other method.
    bool begin(Files *files);

    // Retrieves the value for the given key as String.
    String get(key key);
    // Stores the given value using the given string.
    void put(String string, key key);

    // Copies the value for the given key to the given destination.
    void copy(char *dst, size_t dstsize, key key);

private:
    Files *files;
};

#endif
