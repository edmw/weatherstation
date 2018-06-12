#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Operating Support:
// Class to manage access to the wireless local area network (WiFi).
// This is a no-op if not run on ESP8266.
//
// On connect tries to connect to a previously saved Access Point or opens an own Access Point and
// serves a web configuration portal.
// See https://github.com/tzapu/WiFiManager
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ESP8266
#include <WiFiManager.h>
#endif

#include "Values.h"

class Network {
public:
    Network(String deviceid);

    // Begin managing the WiFi network. Must be called before any other method.
    // [NIY] The given Values manger is used to store additional configuration values.
    bool begin(Values *values);

    // Connects to the WiFi network.
    bool connect(String logger);

    // Connects to the WiFi network with the specified ssid and password.
    bool connect(String ssid, String password);

    // Disconnects from the WiFi network.
    void disconnect(void);

    #ifdef ESP8266
    // Returns a new WiFi client. Only available on ESP8266.
    std::unique_ptr<Client> createClient(void);
    #endif

private:
    String deviceid;

    Values *values;
};

#endif
