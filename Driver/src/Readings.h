#ifndef __READINGS_H__
#define __READINGS_H__

#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Weather Station:
// Class to collect all sensor readings of the Weather Station. A sensor reading consists of
// a measured sensor value and a sensor type. Optionally a sensor identification string can be
// attached. For example (15.9, temperature, "BME280")
///////////////////////////////////////////////////////////////////////////////////////////////////

#if ! defined(__time_t_defined)
typedef unsigned long time_t;
#endif

typedef struct {
    time_t timestamp;
    float voltage;
    float temperature;
    float temperature_alternate;
    float temperature_external;
    float pressure;
    float humidity;
    float humidity_alternate;
    float illuminance;
    float uvintensity;
} readings_t;

class Readings {
public:
    enum reading_type {
        temperature = 0,
        temperature_alternate = 1,
        temperature_external = 2,
        pressure = 3,
        humidity = 4,
        humidity_alternate = 5,
        illuminance = 6,
        uvintensity = 7,
        voltage = 8,
        READING_TYPE_MAX = voltage
    };

    Readings(void);

    // Clears all sensor readings.
    void clear(void);

    // Stores a given sensor reading value for the given sensor reading type.
    void store(float value, reading_type type);
    // Stores a given sensor reading value for the given sensor reading type and attaches the given
    // sensor identification string.
    void store(float value, reading_type type, String sensor_id);

    // Retrieves a stored sensor reading value for the given sensor reading type.
    float retrieve(reading_type type);
    // Retrieves a stored sensor reading value for the given sensor reading type and gives the
    // attached sensor identification string.
    float retrieve(reading_type type, String &sensor_id);

    // Prints all stored sensor readings. Diagnostic method.
    void print(void);

private:
    readings_t readings;

    String sensor_ids[READING_TYPE_MAX + 1];

    void print(reading_type type);

};

#endif
