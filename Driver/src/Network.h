#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Operating Support:
// Class to manage access to the wireless local area network (WiFi).
// This is a no-op if not run on ESP8266 or ESP32.
//
// On connect tries to connect to a previously saved Access Point or opens an own Access Point and
// serves a web configuration portal (ESP8266 only).
// See https://github.com/tzapu/WiFiManager
///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(ESP8266)
#include <WiFiManager.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiClient.h>
#endif

#include "Values.h"

class Network {
public:
    Network(String deviceid);
    Network(String deviceid, String ssid, String sspw);

    // Begin managing the WiFi network. Must be called before any other method.
    // [NIY] The given Values manger is used to store additional configuration values.
    bool begin(Values *values);

    // Connects to the WiFi network.
    bool connect(void);

    // Disconnects from the WiFi network.
    void disconnect(void);

    #if defined(ESP8266) || defined(ESP32)
    // Returns a new WiFi client. Only available on ESP8266 or ESP32.
    std::unique_ptr<Client> createClient(void);
    #endif

private:
    String deviceid;
    String ssid;
    String sspw;

    Values *values;

    bool connect(String ssid, String sspw);
    bool connect(String deviceid);
};

#endif
