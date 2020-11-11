#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Ticker.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiClient.h>
#endif

#include "Network.h"
#include "System.h"

#include "Values.h"
#include "Signaling.h"
#include "Notification.h"

extern const bool PRODUCTION;
extern Signaling signaling;
extern Notification notification;

///////////////////////////////////////////////////////////////////////////////////////////////////

Network::Network(String deviceid)
    : deviceid(deviceid), ssid(""), sspw(""), values(NULL) {
}

Network::Network(String deviceid, String ssid, String sspw)
     : deviceid(deviceid), ssid(ssid), sspw(sspw), values(NULL) {
}

bool Network::begin(Values *values) {
    this->values = values;

    #if defined(ESP8266)
    notification.info(F("*WIFI: HOSTNAME: "), deviceid);
    WiFi.hostname(deviceid.c_str());
    delay(100);
    #elif defined(ESP32)
    notification.info(F("*WIFI: HOSTNAME: "), deviceid);
    WiFi.setHostname(deviceid.c_str());
    delay(100);
    #endif

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(ESP8266)

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

bool Network::connect() {
    System::wifiOn();

    if (connect(ssid, sspw)) {
        return true;
    }

    if (connect(deviceid)) {
        return true;
    }

    return false;
}

bool Network::connect(String ssid, String password) {
    if ((ssid.length() == 0) || (sspw.length() == 0)) { return false; }

    WiFi.begin(ssid.c_str(), password.c_str());
    int status = WiFi.waitForConnectResult();
    if (status != WL_CONNECTED) {
        notification.info(F("*WIFI: status: "), status);
    }
    return status == WL_CONNECTED;
}

bool Network::connect(String deviceid) {
    if (deviceid.length() == 0) { return false; }

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
    wiFiManager.setConnectTimeout(1*60);
    wiFiManager.setConfigPortalTimeout(5*60);

    WiFiManagerParameter wm_influx_secret("Influx", "Influx Shared Secret", influx_secret_buffer, 32);
    wiFiManager.addParameter(&wm_influx_secret);

    bool result = false;

    prepareNetworkForWiFi(&wiFiManager, this);

    if (wiFiManager.autoConnect(deviceid.c_str(), NULL)) {

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
    WiFi.disconnect();
}

std::unique_ptr<Client> Network::createClient(void) {
    return std::unique_ptr<Client>(new WiFiClient());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#elif defined(ESP32)

bool Network::connect() {
    System::wifiOn();

    if (connect(ssid, sspw)) {
        return true;
    }

    return false;
}

bool Network::connect(String ssid, String password) {
    if ((ssid.length() == 0) || (sspw.length() == 0)) { return false; }

    WiFi.begin(ssid.c_str(), password.c_str());
    int status = WiFi.waitForConnectResult();
    if (status != WL_CONNECTED) {
        notification.info(F("*WIFI: status: "), status);
    }
    return status == WL_CONNECTED;
}

void Network::disconnect(void) {
    WiFi.disconnect();
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

bool connect(String ssid, String sspw) {
    return false;
}

bool connect(String deviceid) {
    return false;
}

void Network::disconnect(void) {
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
