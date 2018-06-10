#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Weather Station:
// Class to transport all sensor readings of the Weather Station to a server. The readings will
// be posted to an Influxdb’s REST api.
// This is a no-op if not run on ESP8266.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Network.h"

#include "Readings.h"

class Transport {
public:
    // Constructs a transporter to the given server at the given port for the given database.
    // Logger is an arbitrary identifier for a specific Weather station.
    Transport(String server, int port, String database, String logger, String location);

    // Begin with the given network manager. Must be called before any other method.
    bool begin(Network *network);

    // Sends the given readings to the previously specified server using the previously specified
    // network manager.
    bool send(Readings &readings);

private:
    String server;
    int port;

    String user;
    String token;

    String database;

    String logger;
    String location;

    Network *network;
};

#endif
