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

Network::Network(String deviceid) {
    this->deviceid = deviceid;
    this->values = NULL;
}

bool Network::begin(Values *values) {
    this->values = values;

    WiFi.hostname(deviceid.c_str());
    delay(100);
    WiFi.persistent(false);
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

bool Network::connect(String ssid, String password) {
    WiFi.forceSleepWake();
    delay(100);

    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());
    int status = WiFi.waitForConnectResult();
    if (status != WL_CONNECTED) {
        notification.info(F("*WIFI: status: "), status);
    }
    return status == WL_CONNECTED;
}

bool Network::connect(String logger) {
    WiFi.forceSleepWake();
    delay(100);

    char influx_secret_buffer[34];
    values->copy(influx_secret_buffer, sizeof(influx_secret_buffer), Values::influx_secret);

    WiFiManager wiFiManager;
    if (PRODUCTION) {
        // Production: disable debug output
        wiFiManager.setDebugOutput(false);
    }
    else {
        // Development: print debug output
        notification.info(F("*WIFI: SSID: "), WiFi.SSID());
        notification.info(F("*WIFI: PASS: "), WiFi.psk());
        wiFiManager.setDebugOutput(true);
    }
    wiFiManager.setConnectTimeout(2*60);
    wiFiManager.setConfigPortalTimeout(5*60);

    WiFiManagerParameter wm_influx_secret("Influx", "Influx Shared Secret", influx_secret_buffer, 32);
    wiFiManager.addParameter(&wm_influx_secret);

    bool result = false;

    prepareNetworkForWiFi(&wiFiManager, this);

    if (wiFiManager.autoConnect(logger.c_str(), NULL)) {

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
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(100);
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
