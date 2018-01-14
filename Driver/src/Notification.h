#ifndef __NOTIFICATION_H__
#define __NOTIFICATION_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Operating Support:
// Class to produce output to the serial connection.
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Signaling.h"

class Notification {
public:
    // Constructs a handler for the default serial connection.
    Notification();

    // Begin handling output. Must be called before any other method.
    bool begin(bool production, Signaling *signaling);

    void info(const __FlashStringHelper *message);
    void info(const __FlashStringHelper *message, const String &printable);
    void info(const __FlashStringHelper *message, const char *printable);
    void info(const __FlashStringHelper *message, int printable);
    void info(const __FlashStringHelper *message, unsigned int printable);
    void info(const __FlashStringHelper *message, long printable);
    void info(const __FlashStringHelper *message, unsigned long printable);
    void info(const __FlashStringHelper *message, const Printable &printable);

    void info_millis(const __FlashStringHelper *message, unsigned long millis);

    // Any of the following methods produces output to the serial connection and triggers a signal.

    void warn(const __FlashStringHelper *message);
    void warn(const __FlashStringHelper *message, const String &printable);

    // Any of the following methods produces output to the serial connection, triggers a signal
    // and stops execution.

    void fatal(const __FlashStringHelper *message, uint8_t blink = 0);

private:
    bool production;

    Signaling *signaling;
};

#endif
