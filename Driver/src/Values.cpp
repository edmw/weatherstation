#include <Arduino.h>

#include "Values.h"

#include "Files.h"
#include "Notification.h"

extern const bool PRODUCTION;
extern Notification notification;

///////////////////////////////////////////////////////////////////////////////////////////////////

Values::Values(void) {
    this->files = NULL;
}

bool Values::begin(Files *files) {
    this->files = files;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const String influx_secret_filename = String("influx_secret");

String filename_for_key(Values::key key) {
    switch (key) {
    case Values::influx_secret:
        return influx_secret_filename;
    }
    return String();
}

String Values::get(key key) {
    String filename = filename_for_key(key);
    if (files != NULL) {
        if (files->exists(filename)) {
            return files->load(filename);
        }
    }
    return String();
}

void Values::put(String string, key key) {
    String filename = filename_for_key(key);
    if (files != NULL) {
        files->save(filename, string);
    }
}

void Values::copy(char *dst, size_t dstsize, key key) {
    String string = get(key);
    strlcpy(dst, string.c_str(), dstsize);
}
