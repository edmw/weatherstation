#include <Arduino.h>
#include <ArduinoHttpClient.h>

#include "Transport.h"

#include "Network.h"

#include "Notification.h"

extern const bool PRODUCTION;
extern Notification notification;

///////////////////////////////////////////////////////////////////////////////////////////////////

Transport::Transport(String server, int port, String database, String logger, String location) {
    this->server = server;
    this->port = port;
    this->user = "";
    this->token = "";
    this->database = database;
    this->logger = logger;
    this->location = location;
    this->network = NULL;
}

bool Transport::begin(Network *network) {
    this->network = network;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(ESP8266) || defined(ESP32)

String format_fields(Readings &readings) {
    String fields;

    float voltage0 = readings.retrieve(Readings::voltage);
    if (!isnan(voltage0)) {
        if (fields.length() > 0) fields += ",";
        fields += "voltage0=" + String(voltage0, 4);
    }

    float temperature0 = readings.retrieve(Readings::temperature);
    if (!isnan(temperature0)) {
        if (fields.length() > 0) fields += ",";
        fields += "temperature0=" + String(temperature0, 4);
    }

    float temperature1 = readings.retrieve(Readings::temperature_external);
    if (!isnan(temperature1)) {
        if (fields.length() > 0) fields += ",";
        fields += "temperature1=" + String(temperature1, 4);
    }

    float humidity0 = readings.retrieve(Readings::humidity);
    if (!isnan(humidity0)) {
        if (fields.length() > 0) fields += ",";
        fields += "humidity0=" + String(humidity0, 4);
    }

    float pressure0 = readings.retrieve(Readings::pressure);
    if (!isnan(pressure0)) {
        if (fields.length() > 0) fields += ",";
        fields += "pressure0=" + String(pressure0, 4);
    }

    return fields;
}

bool Transport::send(Readings &readings) {
    bool result = false;

    if (network) {
        std::unique_ptr<Client> networkClient = network->createClient();

        Client *networkClientPtr = networkClient.get();
        if (networkClientPtr) {

            HttpClient httpClient = HttpClient(*networkClientPtr, server, port);

            String measurement = "weather";
            String tag_set = "location=" + location + ",logger=" + logger;
            String field_set = format_fields(readings);

            if (field_set.length() > 0) {
                notification.info(F("*TRANSPORT: database="), database);
                notification.info(F("*TRANSPORT: measurement="), measurement);
                notification.info(F("*TRANSPORT: tag_set="), tag_set);
                notification.info(F("*TRANSPORT: field_set="), field_set);

                String data = measurement + "," + tag_set + " " + field_set + "\n";

                String requestPath = "/write?db=" + database + "&precision=s&user=" + user;
                httpClient.beginRequest();
                httpClient.post(requestPath);
                httpClient.sendHeader("Content-Type", "application/x-www-form-urlencoded");
                httpClient.sendHeader("Content-Length", data.length());
                httpClient.sendHeader("User-Agent", logger);
                httpClient.beginBody();
                httpClient.print(data);
                httpClient.endRequest();

                int statusCode = httpClient.responseStatusCode();
                if (statusCode == 204) {
                    result = true;
                }
                else {
                    notification.warn(F("*TRANSPORT: Failed with status code "), String(statusCode));
                }
            }
            else {
                notification.info(F("*TRANSPORT: Empty field set!"));
                result = true;
            }
        }
        networkClient.release();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#else
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Transport::send(Readings &readings) {
    return false;
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
