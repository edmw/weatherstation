#ifndef __OTA_H__
#define __OTA_H__

#include <Arduino.h>

#include "Network.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Operating Support:
// Class to manage over-the-air updates.
// This is a no-op if not run on ESP8266. Support for ESP32 pending.
///////////////////////////////////////////////////////////////////////////////////////////////////

class OTA {
public:

    OTA();

    // Begin with the given network manager. Must be called before any other method.
    bool begin(void);

    // Performs the update. Needs an active WiFi connection.
    void performUpdate(void);

private:
    Network *network;
};

#endif
