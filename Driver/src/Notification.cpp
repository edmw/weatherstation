#include <Arduino.h>

#include "Notification.h"
#include "Signaling.h"
#include "System.h"

#include "SERIAL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

Notification::Notification() {
    production = false;
    signaling = NULL;
}

bool Notification::begin(bool production, Signaling *signaling) {
    this->production = production;
    this->signaling = signaling;
    info(F("\r\n"));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Notification::info(const __FlashStringHelper *message) {
    if (!production) { SERIAL_PRINTLN(message); }
}
void Notification::info(const __FlashStringHelper *message, const String &printable) {
    if (!production) { SERIAL_PRINT(message); SERIAL_PRINTLN(printable); }
}
void Notification::info(const __FlashStringHelper *message, const char *printable) {
    if (!production) { SERIAL_PRINT(message); SERIAL_PRINTLN(printable); }
}
void Notification::info(const __FlashStringHelper *message, int printable) {
    if (!production) { SERIAL_PRINT(message); SERIAL_PRINTLN(printable); }
}
void Notification::info(const __FlashStringHelper *message, unsigned int printable) {
    if (!production) { SERIAL_PRINT(message); SERIAL_PRINTLN(printable); }
}
void Notification::info(const __FlashStringHelper *message, long printable) {
    if (!production) { SERIAL_PRINT(message); SERIAL_PRINTLN(printable); }
}
void Notification::info(const __FlashStringHelper *message, unsigned long printable) {
    if (!production) { SERIAL_PRINT(message); SERIAL_PRINTLN(printable); }
}
void Notification::info(const __FlashStringHelper *message, const Printable &printable) {
    if (!production) { SERIAL_PRINT(message); SERIAL_PRINTLN(printable); }
}
void Notification::info_millis(const __FlashStringHelper *message, unsigned long millis) {
    if (!production) { SERIAL_PRINT_MILLIS(message, millis); }
}

void Notification::warn(const __FlashStringHelper *message) {
    if (!production) {
        SERIAL_PRINT(F("WARN: ")); SERIAL_PRINTLN(message);
    }
    if (signaling != NULL) {
        signaling->signal_failure_once(500);
    }
}
void Notification::warn(const __FlashStringHelper *message, const String &printable) {
    if (!production) {
        SERIAL_PRINT(F("WARN: ")); SERIAL_PRINT(message); SERIAL_PRINTLN(printable);
    }
    if (signaling != NULL) {
        signaling->signal_failure_once(500);
    }
}

void Notification::fatal(const __FlashStringHelper *message, uint8_t blink, bool forever) {
    SERIAL_PRINT(F("FATAL: ")); SERIAL_PRINTLN(message);
    if (signaling != NULL) {
        if (forever) {
            // blink forever and does never return nor reset
            signaling->signal_failure_count_forever(blink);
        }
        else {
            // blink 10 times then go on with System::fatal
            for (int i = 0; i < 10; i++) {
                signaling->signal_failure_count_once(blink);
            }
        }
    }
    else {
        if (forever) {
            // no-op and does never return nor reset
            while(1) {
                delay(1000);
            }
        }
        else {
            // no-op then go on with System::fatal
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
