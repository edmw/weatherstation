#include <Arduino.h>

#ifdef ESP8266
#include <WiFiManager.h>
#include <Ticker.h>
#endif

#include "Network.h"

#include "Values.h"
#include "Signaling.h"
#include "Notification.h"

extern const bool PRODUCTION;
extern Signaling signaling;
extern Notification notification;

///////////////////////////////////////////////////////////////////////////////////////////////////

Network::Network(void) {
    this->values = NULL;
}

bool Network::begin(Values *values) {
    this->values = values;
    delay(100);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ESP8266

Ticker ticker;

void tick() {
    signaling.signal_toggle();
}

Network *networkToConfig;

bool configChanged;

void startConfigCallback(WiFiManager *wm) {
    ticker.attach(0.2, tick);
}

void saveConfigCallback() {
    configChanged = true;
}

void prepareNetworkForWiFi(WiFiManager *wm, Network *network) {
    networkToConfig = network;
    configChanged = false;
    wm->setAPCallback(startConfigCallback);
    wm->setSaveConfigCallback(saveConfigCallback);
}

void finishNetworkForWiFi(WiFiManager *wm, Network *network) {
    ticker.detach();
    signaling.signal_off();
}

bool Network::connect(void) {
    WiFi.forceSleepWake();

    char influx_secret_buffer[34];
    values->copy(influx_secret_buffer, sizeof(influx_secret_buffer), Values::influx_secret);

    WiFiManager wiFiManager;
    if (PRODUCTION) {
        // Production: disable debug output
        wiFiManager.setDebugOutput(false);
    }
    else {
        // Development: print debug output
        wiFiManager.setDebugOutput(true);
    }
    wiFiManager.setConnectTimeout(1*60);
    wiFiManager.setConfigPortalTimeout(5*60);

    WiFiManagerParameter wm_influx_secret("Influx", "Influx Shared Secret", influx_secret_buffer, 32);
    wiFiManager.addParameter(&wm_influx_secret);

    bool result = false;

    prepareNetworkForWiFi(&wiFiManager, this);

    if (wiFiManager.autoConnect()) {

        strlcpy(influx_secret_buffer, wm_influx_secret.getValue(), 34);
        if (configChanged) {
            values->put(String(influx_secret_buffer), Values::influx_secret);
        }

        result = true;
    }

    finishNetworkForWiFi(&wiFiManager, this);

    return result;
}

void Network::disconnect(void) {
    WiFi.forceSleepBegin();
}

std::unique_ptr<Client> Network::createClient(void) {
    return std::unique_ptr<Client>(new WiFiClient());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#else
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Network::connect(void) {
    return false;
}

void Network::disconnect(void) {
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
