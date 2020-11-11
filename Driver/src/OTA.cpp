#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266httpUpdate.h>
#endif

#include "OTA.h"

#include "Driver.h"
#include "System.h"

#include "Notification.h"

extern const int SKETCH_VERSION;
extern Notification notification;

///////////////////////////////////////////////////////////////////////////////////////////////////

OTA::OTA() {
}

bool OTA::begin() {
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(ESP8266)

void OTA::performUpdate(void) {
    // guard: do not perform an update if waking up after deep sleep
    if (System::lastResetReasonIsDeepSleepAwake()) {
        notification.info(F("Update skipped: Deep Sleep awake"));
        return;
    }

    notification.info(F("Checking update:"), OTA_URL);
    notification.info(F("lastResetReason:"), String(System::lastResetReason()));

    // perform update
    switch(ESPhttpUpdate.update(OTA_URL, String(SKETCH_VERSION))) {
        case HTTP_UPDATE_NO_UPDATES:
            notification.info(F("No update available!"));
            break;
        case HTTP_UPDATE_FAILED:
            notification.warn(F("Update failed: "), ESPhttpUpdate.getLastErrorString());
            break;
        case HTTP_UPDATE_OK:
            // may not be called -> reboot the ESP
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#else
///////////////////////////////////////////////////////////////////////////////////////////////////

void OTA::performUpdate(void) {
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
